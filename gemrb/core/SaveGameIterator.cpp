/* GemRB - Infinity Engine Emulator
 * Copyright (C) 2003 The GemRB Project
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
 *
 */

#include "SaveGameIterator.h"

#include "strrefs.h"

#include "DisplayMessage.h"
#include "GameData.h" // For ResourceHolder
#include "ImageMgr.h"
#include "ImageWriter.h"
#include "Interface.h"
#include "PluginMgr.h"
#include "SaveGameMgr.h"
#include "Sprite2D.h"
#include "TableMgr.h"
#include "GUI/GameControl.h"
#include "Scriptable/Actor.h"
#include "Streams/FileStream.h"

#ifndef R_OK
#define R_OK 04
#endif

#include <cassert>
#include <set>
#include <ctime>

#ifdef VITA
#include <dirent.h>
#endif

namespace GemRB {

const TypeID SaveGame::ID = { "SaveGame" };

/** Extract date from save game ds into Date. */
static std::string ParseGameDate(DataStream *ds)
{
	char Signature[8];
	ieDword GameTime;
	ds->Read(Signature, 8);
	ds->ReadDword(GameTime);
	delete ds;
	if (memcmp(Signature, "GAME", 4) != 0) {
		return "ERROR";
	}

	int hours = ((int)GameTime)/core->Time.hour_sec;
	int days = hours/24;
	hours -= days*24;
	std::string a, b, c;

	// pst has a nice single string for everything 41277 (individual ones lack tokens)
	core->GetTokenDictionary()->SetAtAsString("GAMEDAYS", days);
	core->GetTokenDictionary()->SetAtAsString("HOUR", hours);
	ieStrRef dayref = DisplayMessage::GetStringReference(STR_DAY);
	ieStrRef daysref = DisplayMessage::GetStringReference(STR_DAYS);
	if (dayref == daysref) {
		return core->GetMBString(ieStrRef::DATE2);
	}

	if (days) {
		if (days==1) a = core->GetMBString(dayref, STRING_FLAGS::NONE);
		else a = core->GetMBString(daysref, STRING_FLAGS::NONE);
	}
	if (hours || a.empty()) {
		if (!a.empty()) b=core->GetMBString(ieStrRef::DATE1); // and
		if (hours==1) c = core->GetMBString(DisplayMessage::GetStringReference(STR_HOUR), STRING_FLAGS::NONE);
		else c = core->GetMBString(DisplayMessage::GetStringReference(STR_HOURS), STRING_FLAGS::NONE);
	}
	
	if (!b.empty()) {
		return a + " " + b + " " + c;
	} else {
		return a + c;
	}
}

SaveGame::SaveGame(std::string path, std::string name, const ResRef& prefix, std::string slotname, int pCount, int saveID)
: Path(std::move(path)), Name(std::move(name)), Prefix(prefix), SlotName(std::move(slotname))
{
	PortraitCount = pCount;
	SaveID = saveID;
	char nPath[_MAX_PATH + 1] = {};
	struct stat my_stat;
	PathJoinExt(nPath, Path.c_str(), Prefix.CString(), "bmp");
	memset(&my_stat, 0, sizeof(my_stat));
	if (stat(nPath, &my_stat)) {
		Log(ERROR, "SaveGameIterator", "Stat call failed, using dummy time!");
		Date = "Sun 31 Feb 00:00:01 2099";
	} else {
		strftime(nPath, _MAX_PATH, "%c", localtime(&my_stat.st_mtime));
		Date = nPath;
	}
	manager.AddSource(Path.c_str(), Name.c_str(), PLUGIN_RESOURCE_DIRECTORY);
}

Holder<Sprite2D> SaveGame::GetPortrait(int index) const
{
	if (index > PortraitCount) {
		return NULL;
	}

	std::string nPath = fmt::format("PORTRT{}", index);
	ResourceHolder<ImageMgr> im = GetResourceHolder<ImageMgr>(nPath, manager, true);
	if (!im)
		return NULL;
	return im->GetSprite2D();
}

Holder<Sprite2D> SaveGame::GetPreview() const
{
	ResourceHolder<ImageMgr> im = GetResourceHolder<ImageMgr>(Prefix, manager, true);
	if (!im)
		return NULL;
	return im->GetSprite2D();
}

DataStream* SaveGame::GetGame() const
{
	return manager.GetResource(Prefix, IE_GAM_CLASS_ID, true);
}

DataStream* SaveGame::GetWmap(int idx) const
{
	return manager.GetResource(core->WorldMapName[idx], IE_WMP_CLASS_ID, true);
}

DataStream* SaveGame::GetSave() const
{
	return manager.GetResource(Prefix, IE_SAV_CLASS_ID, true);
}

const std::string& SaveGame::GetGameDate() const
{
	if (GameDate.empty())
		GameDate = ParseGameDate(GetGame());
	return GameDate;
}

// mission pack save dir or the main one?
static std::string SaveDir()
{
	std::string saveDir("save");
	core->GetTokenDictionary()->Lookup("SaveDir", saveDir);
	return saveDir;
}

/*
 * Returns the first 0 bit position of an integer
 */
static int GetHole(int n)
{
	int mask = 1;
	int value = 0;
	while(n&mask) {
		mask<<=1;
		value++;
	}
	return value;
}

/*
 * Returns the age of a quickslot entry. Returns 0 if it isn't a quickslot
 */
static int IsQuickSaveSlot(StringView match, StringView slotname)
{
	char savegameName[_MAX_PATH];
	int savegameNumber = 0;
	int cnt = sscanf(slotname.c_str(), SAVEGAME_DIRECTORY_MATCHER, &savegameNumber, savegameName);
	if (cnt != 2) {
		return 0;
	}
	if (stricmp(savegameName, match.c_str()) != 0)
	{
		return 0;
	}
	return savegameNumber;
}
/*
 * Return true if directory Path/slotname is a potential save game
 * slot, otherwise return false.
 */
static bool IsSaveGameSlot(const char* Path, const char* slotname)
{
	char savegameName[_MAX_PATH];
	int savegameNumber = 0;

	if (slotname[0] == '.')
		return false;

	int cnt = sscanf( slotname, SAVEGAME_DIRECTORY_MATCHER, &savegameNumber, savegameName );
	if (cnt != 2) {
		//The matcher didn't match: either this is not a valid dir
		//or the SAVEGAME_DIRECTORY_MATCHER needs updating.
		Log(ERROR, "SaveGameIterator", "Invalid savegame directory '{}' in {}.",
			slotname, Path);
		return false;
	}

	//The matcher got matched correctly.
	char dtmp[_MAX_PATH];
	PathJoin(dtmp, Path, slotname, nullptr);

	char ftmp[_MAX_PATH];
	PathJoinExt(ftmp, dtmp, core->GameNameResRef.CString(), "bmp");

	if (access( ftmp, R_OK )) {
		Log(WARNING, "SaveGameIterator", "Ignoring slot {} because of no appropriate preview!", dtmp);
		return false;
	}

	PathJoinExt(ftmp, dtmp, core->WorldMapName[0].CString(), "wmp");
	if (access( ftmp, R_OK )) {
		return false;
	}

	if (core->WorldMapName[1]) {
		PathJoinExt(ftmp, dtmp, core->WorldMapName[1].CString(), "wmp");
		if (access(ftmp, R_OK)) {
			Log(WARNING, "SaveGameIterator", "Ignoring slot {} because of no appropriate second worldmap!", dtmp);
			return false;
		}
	}

	return true;
}

bool SaveGameIterator::RescanSaveGames()
{
	// delete old entries
	save_slots.clear();

	char Path[_MAX_PATH];
	PathJoin(Path, core->config.SavePath, SaveDir().c_str(), nullptr);

	DirectoryIterator dir(Path);
	// create the save game directory at first access
	if (!dir) {
		if (!MakeDirectories(Path)) {
			Log(ERROR, "SaveGameIterator", "Unable to create save game directory '{}'", Path);
			return false;
		}
		dir.Rewind();
	}
	if (!dir) { //If we cannot open the Directory
		return false;
	}

	std::set<std::string> slots;
	dir.SetFlags(DirectoryIterator::Directories);
	do {
		const char *name = dir.GetName();
		if (IsSaveGameSlot( Path, name )) {
			slots.emplace(name);
		}
	} while (++dir);

	for (const auto& slot : slots) {
		save_slots.push_back(BuildSaveGame(slot));
	}

	return true;
}

const std::vector<Holder<SaveGame> >& SaveGameIterator::GetSaveGames()
{
	RescanSaveGames();

	return save_slots;
}

Holder<SaveGame> SaveGameIterator::GetSaveGame(StringView name)
{
	RescanSaveGames();

	for (const auto& saveSlot : save_slots) {
		if (saveSlot->GetName().compare(name.c_str()) == 0)
			return saveSlot;
	}
	return NULL;
}

Holder<SaveGame> SaveGameIterator::BuildSaveGame(std::string slotname)
{
	int prtrt = 0;
	char Path[_MAX_PATH];
	//lets leave space for the filenames
	PathJoin(Path, core->config.SavePath, SaveDir().c_str(), slotname.c_str(), nullptr);

	char savegameName[_MAX_PATH]={0};
	int savegameNumber = 0;

	int cnt = sscanf(slotname.c_str(), SAVEGAME_DIRECTORY_MATCHER, &savegameNumber, savegameName);
	//maximum pathlength == 240, without 8+3 filenames
	if ( (cnt != 2) || (strlen(Path)>240) ) {
		Log(WARNING, "SaveGame", "Invalid savegame directory '{}' in {}.", slotname, Path);
		return NULL;
	}

	DirectoryIterator dir(Path);
	if (!dir) {
		return NULL;
	}
	do {
		if (strnicmp( dir.GetName(), "PORTRT", 6 ) == 0)
			prtrt++;
	} while (++dir);

	return MakeHolder<SaveGame>(Path, savegameName, core->GameNameResRef, std::move(slotname), prtrt, savegameNumber);
}

void SaveGameIterator::PruneQuickSave(StringView folder) const
{
	auto FormatQuickSavePath = [folder](int i)
	{
		return fmt::format(FMT_STRING("{}{}{}{:09d}-{}"), core->config.SavePath, SaveDir(), SPathDelimiter, i, folder);
	};

	//storing the quicksave ages in an array
	std::vector<int> myslots;
	for (const auto& saveSlot : save_slots) {
		int tmp = IsQuickSaveSlot(folder, saveSlot->GetSlotName());
		if (tmp) {
			size_t pos = myslots.size();
			while(pos-- && myslots[pos]>tmp) ;
			myslots.insert(myslots.begin()+pos+1,tmp);
		}
	}
	//now we got an integer array in myslots
	size_t size = myslots.size();

	if (!size) {
		return;
	}

	int n=myslots[size-1];
	size_t hole = GetHole(n);
	if (hole<size) {
		//prune second path
		std::string from = FormatQuickSavePath(myslots[hole]);
		myslots.erase(myslots.begin()+hole);
		core->DelTree(from.c_str(), false);
		rmdir(from.c_str());
	}
	//shift paths, always do this, because they are aging
	size = myslots.size();
	for (size_t i = size; i > 0; i--) {
		std::string from = FormatQuickSavePath(myslots[i]);
		std::string to = FormatQuickSavePath(myslots[i]+1);
		int errnum = rename(from.c_str(), to.c_str());
		if (errnum) {
			error("SaveGameIterator", "Rename error {} when pruning quicksaves!", errnum);
		}
	}
}

/** Save game to given directory */
static bool DoSaveGame(const char *Path, bool overrideRunning)
{
	const Game *game = core->GetGame();
	//saving areas to cache currently in memory
	unsigned int mc = (unsigned int) game->GetLoadedMapCount();
	while (mc--) {
		Map *map = game->GetMap(mc);
		if (core->SwapoutArea(map)) {
			return false;
		}
	}

	gamedata->SaveAllStores();

	//compress files in cache named: .STO and .ARE
	//no .CRE would be saved in cache
	if (core->CompressSave(Path, overrideRunning)) {
		return false;
	}

	//Create .gam file from Game() object
	if (core->WriteGame(Path)) {
		return false;
	}

	//Create .wmp file from WorldMap() object
	if (core->WriteWorldMap(Path)) {
		return false;
	}

	PluginHolder<ImageWriter> im = MakePluginHolder<ImageWriter>(PLUGIN_IMAGE_WRITER_BMP);
	if (!im) {
		Log(ERROR, "SaveGameIterator", "Couldn't create the BMPWriter!");
		return false;
	}

	//Create portraits
	for (int i = 0; i < game->GetPartySize( false ); i++) {
		const Actor *actor = game->GetPC( i, false );
		Holder<Sprite2D> portrait = actor->CopyPortrait(true);

		if (portrait) {
			std::string fname = fmt::format("PORTRT{}", i);
			FileStream outfile;
			outfile.Create(Path, fname.c_str(), IE_BMP_CLASS_ID);
			// NOTE: we save the true portrait size, even tho the preview buttons arent (always) the same
			// we do this because: 1. the GUI should be able to use whatever size it wants
			// and 2. its more appropriate to have a flag on the buttons to do the scaling/cropping
			im->PutImage(&outfile, portrait);
		}
	}

	// Create area preview
	// FIXME: the preview should be passed in by the caller!

	WindowManager* wm = core->GetWindowManager();
	Holder<Sprite2D> preview = wm->GetScreenshot(wm->GetGameWindow());

	// scale down to get more of the screen and reduce the size
	preview = core->GetVideoDriver()->SpriteScaleDown(preview, 5);
	FileStream outfile;
	outfile.Create( Path, core->GameNameResRef.CString(), IE_BMP_CLASS_ID );
	im->PutImage( &outfile, preview );

	return true;
}

static int CanSave()
{
	//some of these restrictions might not be needed
	// NOTE: can't save  during a rest, chapter information or movie (ref CANTSAVEMOVIE)
	// is handled automatically, but without a message
	if (core->InCutSceneMode()) {
		displaymsg->DisplayConstantString(STR_CANTSAVE, GUIColors::XPCHANGE);
		return 1;
	}

	const Store *store = core->GetCurrentStore();
	if (store) {
		displaymsg->DisplayConstantString(STR_CANTSAVESTORE, GUIColors::XPCHANGE);
		return 1; //can't save while store is open
	}
	const GameControl *gc = core->GetGameControl();
	if (!gc) {
		displaymsg->DisplayConstantString(STR_CANTSAVE, GUIColors::XPCHANGE);
		return -1; //no gamecontrol!!!
	}
	if (gc->GetDialogueFlags()&DF_IN_DIALOG) {
		displaymsg->DisplayConstantString(STR_CANTSAVEDIALOG, GUIColors::XPCHANGE);
		return 2; //can't save while in dialog
	}

	const Game *game = core->GetGame();
	if (!game) {
		displaymsg->DisplayConstantString(STR_CANTSAVE, GUIColors::XPCHANGE);
		return -1;
	}
	if (game->CombatCounter) {
		displaymsg->DisplayConstantString(STR_CANTSAVECOMBAT, GUIColors::XPCHANGE);
		return 3;
	}

	const Map *map = game->GetCurrentArea();
	if (!map) {		
		displaymsg->DisplayConstantString(STR_CANTSAVE, GUIColors::XPCHANGE);
		return -1;
	}

	// hopefully not too strict — we check for any projectile, not just repeating AOE
	proIterator pIter;
	if (map->GetProjectileCount(pIter)) {
		// can't save while AOE spells are in effect
		displaymsg->DisplayConstantString(STR_CANTSAVE, GUIColors::XPCHANGE);
	}

	if (map->AreaFlags&AF_NOSAVE) {
		//cannot save in area
		displaymsg->DisplayConstantString(STR_CANTSAVE, GUIColors::XPCHANGE);
		return 4;
	}

	int i = game->GetPartySize(true);
	while(i--) {
		const Actor *actor = game->GetPC(i, true);
		// can't save while (party) actors are in helpless or dead states
		// STATE_NOSAVE tracks actors not to be stored in GAM, not game saveability
		if (actor->GetStat(IE_STATE_ID) & (STATE_NOSAVE|STATE_MINDLESS)) {
			//some actor is in nosave state
			displaymsg->DisplayConstantString(STR_CANTSAVENOCTRL, GUIColors::XPCHANGE);
			return 5;
		}
		if (actor->GetCurrentArea()!=map) {
			//scattered
			displaymsg->DisplayConstantString(STR_CANTSAVE, GUIColors::XPCHANGE);
			return 6;
		}
		if (map->AnyEnemyNearPoint(actor->Pos)) {
			displaymsg->DisplayConstantString( STR_CANTSAVEMONS, GUIColors::XPCHANGE );
			return 7;
		}
	}

	Point pc1 =  game->GetPC(0, true)->Pos;
	std::vector<Actor *> nearActors = map->GetAllActorsInRadius(pc1, GA_NO_DEAD|GA_NO_UNSCHEDULED, 15);
	for (const auto& neighbour : nearActors) {
		if (neighbour->GetInternalFlag() & IF_NOINT) {
			// dialog about to start or similar
			displaymsg->DisplayConstantString(STR_CANTSAVEDIALOG2, GUIColors::XPCHANGE);
			return 8;
		}
	}

	return 0;
}

static bool CreateSavePath(char *Path, int index, StringView slotname)
{
	PathJoin(Path, core->config.SavePath, SaveDir().c_str(), nullptr);

	//if the path exists in different case, don't make it again
	if (!MakeDirectory(Path)) {
		Log(ERROR, "SaveGameIterator", "Unable to create save game directory '{}'", Path);
		return false;
	}
	//keep the first part we already determined existing

	std::string dir = fmt::format("{:09d}-{}", index, slotname);
	PathJoin(Path, Path, dir.c_str(), nullptr);
	//this is required in case the old slot wasn't recognised but still there
	core->DelTree(Path, false);
	if (!MakeDirectory(Path)) {
		Log(ERROR, "SaveGameIterator", "Unable to create save game directory '{}'", Path);
		return false;
	}
	return true;
}

int SaveGameIterator::CreateSaveGame(int index, bool mqs) const
{
	AutoTable tab = gamedata->LoadTable("savegame");
	StringView slotname;
	int qsave = 0;

	if (tab) {
		slotname = tab->QueryField(index);
		qsave = tab->QueryFieldSigned<int>(index, 1);
	}

	if (mqs) {
		assert(qsave);
		PruneQuickSave(slotname);
	}

	if (int cansave = CanSave())
		return cansave;

	bool overrideRunning = false;
	//if index is not an existing savegame, we create a unique slotname
	for (const auto& save : save_slots) {
		if (save->GetSaveID() != index) continue;

		if (core->saveGameAREExtractor.isRunningSaveGame(*save)) {
			overrideRunning = true;
			if (core->saveGameAREExtractor.createCacheBlob() == GEM_ERROR) {
				return GEM_ERROR;
			}
		}

		DeleteSaveGame(save);
		break;
	}
	char Path[_MAX_PATH];
	GameControl *gc = core->GetGameControl();
	assert(gc);
	if (!CreateSavePath(Path, index, slotname)) {
		displaymsg->DisplayConstantString(STR_CANTSAVE, GUIColors::XPCHANGE);
		gc->SetDisplayText(STR_CANTSAVE, 30);
		return GEM_ERROR;
	}

	if (!DoSaveGame(Path, overrideRunning)) {
		displaymsg->DisplayConstantString(STR_CANTSAVE, GUIColors::XPCHANGE);
		gc->SetDisplayText(STR_CANTSAVE, 30);
		return GEM_ERROR;
	}

	// Save successful / Quick-save successful
	if (qsave) {
		displaymsg->DisplayConstantString(STR_QSAVESUCCEED, GUIColors::XPCHANGE);
		gc->SetDisplayText(STR_QSAVESUCCEED, 30);
	} else {
		displaymsg->DisplayConstantString(STR_SAVESUCCEED, GUIColors::XPCHANGE);
		gc->SetDisplayText(STR_SAVESUCCEED, 30);
	}
	return GEM_OK;
}

int SaveGameIterator::CreateSaveGame(Holder<SaveGame> save, StringView slotname, bool force) const
{
	if (!slotname) {
		return GEM_ERROR;
	}

	int cannotSave = CanSave();
	if (cannotSave && !force) {
		return cannotSave;
	}

	int index;
	bool overrideRunning = false;

	if (save) {
		index = save->GetSaveID();
		if (core->saveGameAREExtractor.isRunningSaveGame(*save)) {
			overrideRunning = true;

			if (core->saveGameAREExtractor.createCacheBlob() == GEM_ERROR) {
				return GEM_ERROR;
			}
		}

		DeleteSaveGame(save);
		save.release();
	} else {
		//leave space for autosaves
		//probably the hardcoded slot names should be read by this object
		//in that case 7 == size of hardcoded slot names array (savegame.2da)
		index = 7;
		for (const auto& save2 : save_slots) {
			if (save2->GetSaveID() >= index) {
				index = save2->GetSaveID() + 1;
			}
		}
	}

	GameControl *gc = core->GetGameControl();
	assert(gc); //this is already checked in CanSave and core only has one if there is a game anyway
	char Path[_MAX_PATH];
	if (!CreateSavePath(Path, index, slotname)) {
		displaymsg->DisplayConstantString(STR_CANTSAVE, GUIColors::XPCHANGE);
		gc->SetDisplayText(STR_CANTSAVE, 30);
		return GEM_ERROR;
	}

	if (!DoSaveGame(Path, overrideRunning)) {
		displaymsg->DisplayConstantString(STR_CANTSAVE, GUIColors::XPCHANGE);
		gc->SetDisplayText(STR_CANTSAVE, 30);
		return GEM_ERROR;
	}

	// Save successful
	displaymsg->DisplayConstantString(STR_SAVESUCCEED, GUIColors::XPCHANGE);
	gc->SetDisplayText(STR_SAVESUCCEED, 30);
	return GEM_OK;
}

void SaveGameIterator::DeleteSaveGame(const Holder<SaveGame>& game) const
{
	if (!game) {
		return;
	}

	core->DelTree(game->GetPath().c_str(), false); //remove all files from folder
	rmdir(game->GetPath().c_str());
}

}
