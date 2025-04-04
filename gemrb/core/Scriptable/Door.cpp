/* GemRB - Infinity Engine Emulator
 * Copyright (C) 2003-2005 The GemRB Project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "Scriptable/Door.h"

#include "strrefs.h"

#include "DisplayMessage.h"
#include "Game.h"
#include "GameData.h"
#include "Projectile.h"
#include "TileMap.h"
#include "GameScript/GSUtils.h"
#include "GUI/GameControl.h"
#include "Scriptable/InfoPoint.h"

namespace GemRB {

#define YESNO(x) ( (x)?"Yes":"No")

DoorTrigger::DoorTrigger(std::shared_ptr<Gem_Polygon> openTrigger, WallPolygonGroup&& openWalls,
			std::shared_ptr<Gem_Polygon> closedTrigger, WallPolygonGroup&& closedWalls)
: openWalls(std::move(openWalls)), closedWalls(std::move(closedWalls)),
openTrigger(std::move(openTrigger)), closedTrigger(std::move(closedTrigger))
{}

void DoorTrigger::SetState(bool open)
{
	isOpen = open;
	for (const auto& wp : openWalls) {
		wp->SetDisabled(!isOpen);
	}
	for (const auto& wp : closedWalls) {
		wp->SetDisabled(isOpen);
	}
}

std::shared_ptr<Gem_Polygon> DoorTrigger::StatePolygon() const
{
	return StatePolygon(isOpen);
}

std::shared_ptr<Gem_Polygon> DoorTrigger::StatePolygon(bool open) const
{
	return open ? openTrigger : closedTrigger;
}

Door::Door(Holder<TileOverlay> Overlay, DoorTrigger&& trigger)
: Highlightable( ST_DOOR ), overlay(std::move(Overlay)), doorTrigger(std::move(trigger))
{
}

void Door::ImpedeBlocks(const std::vector<Point> &points, PathMapFlags value) const
{
	for (const Point& point : points) {
		PathMapFlags tmp = area->tileProps.QuerySearchMap(point) & PathMapFlags::NOTDOOR;
		area->tileProps.SetSearchMap(point, tmp|value);
	}
}

void Door::UpdateDoor()
{
	doorTrigger.SetState(Flags&DOOR_OPEN);
	outline = doorTrigger.StatePolygon();

	if (outline) {
		// update the Scriptable position
		Pos.x = outline->BBox.x + outline->BBox.w/2;
		Pos.y = outline->BBox.y + outline->BBox.h/2;
	}

	PathMapFlags pmdflags;

	if (Flags & DOOR_TRANSPARENT) {
		pmdflags = PathMapFlags::DOOR_IMPASSABLE;
	} else {
		//both door flags are needed here, one for transparency the other
		//is for passability
		pmdflags = PathMapFlags::DOOR_OPAQUE|PathMapFlags::DOOR_IMPASSABLE;
	}
	if (Flags &DOOR_OPEN) {
		ImpedeBlocks(closed_ib, PathMapFlags::IMPASSABLE);
		ImpedeBlocks(open_ib, pmdflags);
	}
	else {
		ImpedeBlocks(open_ib, PathMapFlags::IMPASSABLE);
		ImpedeBlocks(closed_ib, pmdflags);
	}

	InfoPoint *ip = area->TMap->GetInfoPoint(LinkedInfo);
	if (ip) {
		if (Flags&DOOR_OPEN) ip->Flags&=~INFO_DOOR;
		else ip->Flags|=INFO_DOOR;
	}
}

void Door::ToggleTiles(int State, int playsound)
{
	int state;

	if (State) {
		state = !closedIndex;
		if (playsound && !OpenSound.IsEmpty()) {
			core->GetAudioDrv()->PlayRelative(OpenSound, SFX_CHAN_ACTIONS);
		}
	} else {
		state = closedIndex;
		if (playsound && !CloseSound.IsEmpty()) {
			core->GetAudioDrv()->PlayRelative(CloseSound, SFX_CHAN_ACTIONS);
		}
	}
	for (const auto& tile : tiles) {
		overlay->tiles[tile].tileIndex = (ieByte) state;
	}

	//set door_open as state
	Flags = (Flags & ~DOOR_OPEN) | (State == !core->HasFeature(GF_REVERSE_DOOR) );
}

//this is the short name (not the scripting name)
void Door::SetName(const ResRef &name)
{
	ID = name;
}

void Door::SetTiles(std::vector<ieWord> Tiles)
{
	tiles = std::move(Tiles);
}

bool Door::CanDetectTrap() const
{
	// Traps can be detected on all types of infopoint, as long
	// as the trap is detectable and isn't deactivated.
	return (Flags & DOOR_DETECTABLE) && Trapped;
}

void Door::SetDoorLocked(int Locked, int playsound)
{
	if (Locked) {
		if (Flags & DOOR_LOCKED) return;
		Flags|=DOOR_LOCKED;
		// only close it in pst, needed for Dead nations (see 4a3e1cb4ef)
		if (core->HasFeature(GF_REVERSE_DOOR)) SetDoorOpen(false, playsound, 0);
		if (playsound && !LockSound.IsEmpty())
			core->GetAudioDrv()->PlayRelative(LockSound, SFX_CHAN_ACTIONS);
	}
	else {
		if (!(Flags & DOOR_LOCKED)) return;
		Flags&=~DOOR_LOCKED;
		if (playsound && !UnLockSound.IsEmpty())
			core->GetAudioDrv()->PlayRelative(UnLockSound, SFX_CHAN_ACTIONS);
	}
}

int Door::IsOpen() const
{
	int ret = core->HasFeature(GF_REVERSE_DOOR);
	if (Flags&DOOR_OPEN) {
		ret = !ret;
	}
	return ret;
}

bool Door::HitTest(const Point& p) const
{
	if (Flags&DOOR_HIDDEN) {
		return false;
	}

	auto doorpoly = doorTrigger.StatePolygon();
	if (doorpoly) {
		if (!doorpoly->PointIn(p)) return false;
	} else if (Flags&DOOR_OPEN) {
		if (!OpenBBox.PointInside(p)) return false;
	} else {
		if (!ClosedBBox.PointInside(p)) return false;
	}

	return true;
}

std::shared_ptr<Gem_Polygon> Door::OpenTriggerArea() const
{
	return doorTrigger.StatePolygon(true);
}

std::shared_ptr<Gem_Polygon> Door::ClosedTriggerArea() const
{
	return doorTrigger.StatePolygon(false);
}

//also mark actors to fix position
bool Door::BlockedOpen(int Open, int ForceOpen) const
{
	const std::vector<Point> *points = Open ? &open_ib : &closed_ib;
	bool blocked = false;

	//getting all impeded actors flagged for jump
	Region rgn;
	rgn.w = 16;
	rgn.h = 12;
	for(const Point& p : *points) {
		rgn.origin = Map::ConvertCoordFromTile(p);
		PathMapFlags tmp = area->tileProps.QuerySearchMap(p) & PathMapFlags::ACTOR;
		if (tmp != PathMapFlags::IMPASSABLE) {
			auto actors = area->GetActorsInRect(rgn, GA_NO_DEAD|GA_NO_UNSCHEDULED);
			for (Actor* actor : actors) {
				if (actor->GetBase(IE_DONOTJUMP)) {
					continue;
				}
				actor->SetBase(IE_DONOTJUMP, DNJ_JUMP);
				blocked = true;
			}
		}
	}

	if ((Flags&DOOR_SLIDE) || ForceOpen) {
		return false;
	}
	return blocked;
}

void Door::SetDoorOpen(int Open, int playsound, ieDword openerID, bool addTrigger)
{
	if (playsound) {
		//the door cannot be blocked when opening,
		//but the actors will be pushed
		//BlockedOpen will mark actors to be pushed
		if (BlockedOpen(Open,0) && !Open) {
			//clear up the blocking actors
			area->JumpActors(false);
			return;
		}
		area->JumpActors(true);
	}
	if (Open) {
		if (addTrigger) {
			if (Trapped) {
				AddTrigger(TriggerEntry(trigger_opened, openerID));
			} else {
				AddTrigger(TriggerEntry(trigger_harmlessopened, openerID));
			}
		}

		// in PS:T, opening a door does not unlock it
		if (!core->HasFeature(GF_REVERSE_DOOR)) {
			SetDoorLocked(false,playsound);
		}
	} else if (addTrigger) {
		if (Trapped) {
			AddTrigger(TriggerEntry(trigger_closed, openerID));
		} else {
			AddTrigger(TriggerEntry(trigger_harmlessclosed, openerID));
		}
	}
	ToggleTiles(Open, playsound);
	//synchronising other data with the door state
	UpdateDoor();

	core->SetEventFlag(EF_TARGETMODE);
}

bool Door::TryUnlock(Actor *actor) const
{
	if (!(Flags&DOOR_LOCKED)) return true;

	// don't remove key in PS:T!
	bool removekey = !core->HasFeature(GF_REVERSE_DOOR) && Flags&DOOR_KEY;
	return Highlightable::TryUnlock(actor, removekey);
}

void Door::TryDetectSecret(int skill, ieDword actorID)
{
	if (Type != ST_DOOR) return;
	if (Visible()) return;
	if (skill > (signed)DiscoveryDiff) {
		Flags |= DOOR_FOUND;
		core->PlaySound(DS_FOUNDSECRET, SFX_CHAN_HITS);
		AddTrigger(TriggerEntry(trigger_detected, actorID));
	}
}

// return true if the door isn't secret or if it is, but was already discovered
bool Door::Visible() const
{
	return (!(Flags & DOOR_SECRET) || (Flags & DOOR_FOUND)) && !(Flags & DOOR_HIDDEN);
}

void Door::SetNewOverlay(Holder<TileOverlay> Overlay) {
	overlay = std::move(Overlay);
	ToggleTiles(IsOpen(), false);
}

void Highlightable::SetTrapDetected(int x)
{
	if(x == TrapDetected)
		return;
	TrapDetected = x;
	if(TrapDetected) {
		core->PlaySound(DS_FOUNDSECRET, SFX_CHAN_HITS);
		core->Autopause(AUTOPAUSE::TRAP, this);
	}
}

void Highlightable::TryDisarm(const Actor *actor)
{
	if (!Trapped || !TrapDetected) return;

	int skill = actor->GetStat(IE_TRAPS);
	int roll = 0;
	int bonus = 0;
	int trapDC = TrapRemovalDiff;

	if (core->HasFeature(GF_3ED_RULES)) {
		skill = actor->GetSkill(IE_TRAPS);
		roll = core->Roll(1, 20, 0);
		bonus = actor->GetAbilityBonus(IE_INT);
		trapDC = TrapRemovalDiff/7 + 10; // oddity from the original
		if (skill == 0) { // a trained skill
			trapDC = 100;
		}
	} else {
		roll = core->Roll(1, skill/2, 0);
		skill /= 2;
	}

	int check = skill + roll + bonus;
	if (check > trapDC) {
		AddTrigger(TriggerEntry(trigger_disarmed, actor->GetGlobalID()));
		//trap removed
		Trapped = 0;
		if (core->HasFeature(GF_3ED_RULES)) {
			// ~Successful Disarm Device - d20 roll %d + Disarm Device skill %d + INT mod %d >= Trap DC %d~
			displaymsg->DisplayRollStringName(ieStrRef::ROLL6, GUIColors::LIGHTGREY, actor, roll, skill-bonus, bonus, trapDC);
		}
		displaymsg->DisplayConstantStringName(STR_DISARM_DONE, GUIColors::LIGHTGREY, actor);
		int xp = gamedata->GetXPBonus(XP_DISARM, actor->GetXPLevel(1));
		const Game *game = core->GetGame();
		game->ShareXP(xp, SX_DIVIDE);
		core->GetGameControl()->ResetTargetMode();
		core->PlaySound(DS_DISARMED, SFX_CHAN_HITS);
	} else {
		AddTrigger(TriggerEntry(trigger_disarmfailed, actor->GetGlobalID()));
		if (core->HasFeature(GF_3ED_RULES)) {
			// ~Failed Disarm Device - d20 roll %d + Disarm Device skill %d + INT mod %d >= Trap DC %d~
			displaymsg->DisplayRollStringName(ieStrRef::ROLL6, GUIColors::LIGHTGREY, actor, roll, skill-bonus, bonus, trapDC);
		}
		displaymsg->DisplayConstantStringName(STR_DISARM_FAIL, GUIColors::LIGHTGREY, actor);
		TriggerTrap(skill, actor->GetGlobalID());
	}
	ImmediateEvent();
}

void Door::TryPickLock(const Actor *actor)
{
	if (LockDifficulty == 100) {
		if (OpenStrRef != ieStrRef::INVALID) {
			displaymsg->DisplayStringName(OpenStrRef, GUIColors::XPCHANGE, actor, STRING_FLAGS::SOUND | STRING_FLAGS::SPEECH);
		} else {
			displaymsg->DisplayConstantStringName(STR_DOOR_NOPICK, GUIColors::XPCHANGE, actor);
		}
		return;
	}
	int stat = actor->GetStat(IE_LOCKPICKING);
	if (core->HasFeature(GF_3ED_RULES)) {
		int skill = actor->GetSkill(IE_LOCKPICKING);
		if (skill == 0) { // a trained skill, make sure we fail
			stat = 0;
		} else {
			stat *= 7; // convert to percent (magic 7 is from RE)
			int dexmod = actor->GetAbilityBonus(IE_DEX);
			stat += dexmod; // the original didn't use it, so let's not multiply it
			displaymsg->DisplayRollStringName(ieStrRef::ROLL11, GUIColors::LIGHTGREY, actor, stat-dexmod, LockDifficulty, dexmod);
		}
	}
	if (stat < (signed)LockDifficulty) {
		displaymsg->DisplayConstantStringName(STR_LOCKPICK_FAILED, GUIColors::XPCHANGE, actor);
		AddTrigger(TriggerEntry(trigger_picklockfailed, actor->GetGlobalID()));
		core->PlaySound(DS_PICKFAIL, SFX_CHAN_HITS);
		return;
	}
	SetDoorLocked( false, true);
	core->GetGameControl()->ResetTargetMode();
	displaymsg->DisplayConstantStringName(STR_LOCKPICK_DONE, GUIColors::LIGHTGREY, actor);
	AddTrigger(TriggerEntry(trigger_unlocked, actor->GetGlobalID()));
	core->PlaySound(DS_PICKLOCK, SFX_CHAN_HITS);
	ImmediateEvent();
	int xp = gamedata->GetXPBonus(XP_LOCKPICK, actor->GetXPLevel(1));
	const Game *game = core->GetGame();
	game->ShareXP(xp, SX_DIVIDE);
}

void Door::TryBashLock(Actor *actor)
{
	//Get the strength bonus against lock difficulty
	int bonus;
	unsigned int roll;

	if (core->HasFeature(GF_3ED_RULES)) {
		bonus = actor->GetAbilityBonus(IE_STR);
		roll = actor->LuckyRoll(1, 100, bonus, 0);
	} else {
		int str = actor->GetStat(IE_STR);
		int strEx = actor->GetStat(IE_STREXTRA);
		bonus = core->GetStrengthBonus(2, str, strEx); //BEND_BARS_LIFT_GATES
		roll = actor->LuckyRoll(1, 10, bonus, 0);
	}

	actor->FaceTarget(this);
	if (core->HasFeature(GF_3ED_RULES)) {
		// ~Bash door check. Roll %d + %d Str mod > %d door DC.~
		displaymsg->DisplayRollStringName(ieStrRef::ROLL1, GUIColors::LIGHTGREY, actor, roll, bonus, LockDifficulty);
	}

	if(roll < LockDifficulty || LockDifficulty == 100) {
		displaymsg->DisplayConstantStringName(STR_DOORBASH_FAIL, GUIColors::XPCHANGE, actor);
		return;
	}

	displaymsg->DisplayConstantStringName(STR_DOORBASH_DONE, GUIColors::LIGHTGREY, actor);
	SetDoorLocked(false, true);
	core->GetGameControl()->ResetTargetMode();
	Flags|=DOOR_BROKEN;

	//This is ok, bashdoor also sends the unlocked trigger
	AddTrigger(TriggerEntry(trigger_unlocked, actor->GetGlobalID()));
	ImmediateEvent();
}

// returns the appropriate cursor over a door
int Door::GetCursor(int targetMode, int lastCursor) const
{
	if (!Visible()) {
		if (targetMode == TARGET_MODE_NONE) {
			// most secret doors are in walls, so default to the blocked cursor to not give them away
			// iwd ar6010 table/door/puzzle is walkable, secret and undetectable
			return area->GetCursor(Pos);
		} else {
			return lastCursor | IE_CURSOR_GRAY;
		}
	}

	if (targetMode == TARGET_MODE_PICK) {
		if (VisibleTrap(0)) {
			return IE_CURSOR_TRAP;
		}
		if (Flags & DOOR_LOCKED) {
			return IE_CURSOR_LOCK;
		}
		return IE_CURSOR_STEALTH | IE_CURSOR_GRAY;
	}

	return Cursor;
}

std::string Door::dump() const
{
	std::string buffer;
	AppendFormat(buffer, "Debugdump of Door {}:\n", GetScriptName() );
	AppendFormat(buffer, "Door Global ID: {}\n", GetGlobalID());
	AppendFormat(buffer, "Position: {}\n", Pos);
	AppendFormat(buffer, "Door Open: {}\n", YESNO(IsOpen()));
	AppendFormat(buffer, "Door Locked: {}	Difficulty: {}\n", YESNO(Flags&DOOR_LOCKED), LockDifficulty);
	AppendFormat(buffer, "Door Trapped: {}	Difficulty: {}\n", YESNO(Trapped), TrapRemovalDiff);
	if (Trapped) {
		AppendFormat(buffer, "Trap Permanent: {} Detectable: {}\n", YESNO(Flags&DOOR_RESET), YESNO(Flags&DOOR_DETECTABLE) );
	}
	AppendFormat(buffer, "Secret door: {} (Found: {})\n", YESNO(Flags&DOOR_SECRET),YESNO(Flags&DOOR_FOUND));
	ResRef name = "NONE";
	if (Scripts[0]) {
		name = Scripts[0]->GetName();
	}
	AppendFormat(buffer, "Script: {}, Key ({}) removed: {}, Dialog: {}\n", name, KeyResRef, YESNO(Flags&DOOR_KEY), Dialog);
	Log(DEBUG, "Door", "{}", buffer);
	return buffer;
}


}
