# GemRB - Infinity Engine Emulator
# Copyright (C) 2003 The GemRB Project
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#
#
# Some small attempt to re-use code in character generation.
# Attempting to emulate lynx's loop method from BG2 GUI

# Should be called CharGenCommon for continuity but I started
# this with something else in mind and then changed it
# - Fyorl

# CharOverview.py (GUICG)

import GemRB
from GUIDefines import *
from ie_stats import *

CharGenWindow = 0
TextAreaControl = 0
PortraitButton = 0
StepButtons = {}
PersistButtons = {}
Steps = ['Gender', 'Race', 'Class', 'Alignment', 'Abilities', 'Enemy', 'Appearance', 'Name']
GlobalStep = 0

### Utility functions
def AddText(strref, newlines=0):
	TextAreaControl.Append (strref)
	TextAreaControl.Append ("\n" * newlines)
### End utility functions

def PositionCharGenWin(window, offset = 0):
	global CharGenWindow

	CGFrame = CharGenWindow.GetFrame()
	WFrame = window.GetFrame()
	window.SetPos(CGFrame['x'] + CGFrame['w'] - WFrame['w'] - 18,
				  offset + CGFrame['y'] + (CGFrame['h'] - WFrame['h'] - 24))

def UpdateOverview(CurrentStep):
	global CharGenWindow, TextAreaControl, PortraitButton
	global StepButtons, Steps, PersistButtons, GlobalStep
	
	GlobalStep = CurrentStep
	
	CharGenWindow = GemRB.LoadWindow(0, "GUICG")
	CharGenWindow.SetFlags (IE_GUI_VIEW_IGNORE_EVENTS, OP_OR)

	PortraitButton = CharGenWindow.GetControl(12)
	PortraitButton.SetFlags(IE_GUI_BUTTON_PICTURE|IE_GUI_BUTTON_NO_IMAGE,OP_SET)
	
	# Handle portrait
	PortraitName = GemRB.GetToken('LargePortrait')
	if PortraitName != '' and CurrentStep > 1:
		PortraitButton.SetPicture(PortraitName, 'NOPORTLG')
	
	# Handle step buttons
	TextLookup = [11956, 11957, 11959, 11958, 11960, 11983, 11961, 11963]
	for i, Step in enumerate(Steps):
		StepButtons[Step] = CharGenWindow.GetControl(i)
		StepButtons[Step].SetText(TextLookup[i])
		State = IE_GUI_BUTTON_DISABLED
		if CurrentStep - 1 == i:
			State = IE_GUI_BUTTON_ENABLED
			StepButtons[Step].MakeDefault()
			StepButtons[Step].OnPress (NextPress)
		StepButtons[Step].SetState(State)
	
	# Handle (not so) persistent buttons
	# This array handles all the default values for the buttons
	# Exceptions are handled within the loop
	ControlLookup = {
		'Bio': [16, 18003, 0, None],
		'Import': [13, 13955, 0, None],
		'Back': [11, 15416, 1, BackPress],
		'Next': [8, 28210, 0, None],
		'Start': [15, 36788, 1, StartOver]
	}
	States = [IE_GUI_BUTTON_DISABLED, IE_GUI_BUTTON_ENABLED]
	for Key in ControlLookup:
		PersistButtons[Key] = CharGenWindow.GetControl(ControlLookup[Key][0])
		Text = ControlLookup[Key][1]
		State = States[ControlLookup[Key][2]]
		Event = ControlLookup[Key][3]
		
		if Key == 'Bio' and CurrentStep == 9:
			State = States[1]
			import CharGen9
			Event = CharGen9.BioPress
		
		if Key == 'Import' and CurrentStep == 1:
			State = States[1]
			Event = ImportPress
		
		if Key == 'Back':
			if CurrentStep == 1:
				State = States[0]
				Event = None
			else:
				PersistButtons[Key].MakeEscape()
		
		if Key == 'Next' and CurrentStep == 9:
			State = 1
			Event = NextPress
			PersistButtons[Key].MakeDefault()
		
		if Key == 'Start' and CurrentStep == 1:
			Text = 13727
			Event = CancelPress
			PersistButtons[Key].MakeEscape()
		
		PersistButtons[Key].SetText(Text)
		PersistButtons[Key].SetState(State)
		
		if Event:
			PersistButtons[Key].OnPress (Event)
	
	# Handle character overview information
	TextAreaControl = CharGenWindow.GetControl(9)
	Tables = []
	for tbl in ['races', 'classes', 'aligns', 'ability', 'skillsta', 'skills', 'featreq', 'feats']:
		Tables.append(GemRB.LoadTable(tbl))
	
	if GemRB.GetVar('Gender') > 0:
		name = GemRB.GetToken('CHARNAME')
		if not name:
			TextAreaControl.SetText(12135)
		else:
			TextAreaControl.SetText(1047)
			AddText(': ' + GemRB.GetToken('CHARNAME'), 1)
			AddText(12135)
		AddText(': ')
		strref = 1049 + GemRB.GetVar('Gender')
		AddText(strref, 1)
	
	if GemRB.GetVar('Race') > 0:
		AddText(1048)
		AddText(': ')
		AddText(Tables[0].GetValue(Tables[0].FindValue(3, GemRB.GetVar('Race')), 2), 1)
	
	if GemRB.GetVar('Class') > 0:
		AddText(11959)
		AddText(': ')
		AddText(Tables[1].GetValue(GemRB.GetVar('Class') - 1, 0), 1)
	
	if GemRB.GetVar('Alignment') > 0:
		AddText(11958)
		AddText(': ')
		AddText(Tables[2].GetValue(GemRB.GetVar('Alignment') - 1, 0), 1)
	
	if GemRB.GetVar('Ability 0') > 0:
		AddText('\n[color=FFFF00]' + GemRB.GetString(17088) + '[/color]', 1)
		for i in range(0, 6):
			strref = Tables[3].GetValue(i, 2)
			AddText(strref)
			abl = GemRB.GetVar('Ability ' + str(i))
			AddText(': %d (%+d)' % (abl, abl / 2 - 5), 1)
	
	if CurrentStep > 6:
		AddText('\n[color=FFFF00]' + GemRB.GetString(11983) + '[/color]', 1)
		
		ClassColumn = Tables[1].GetValue(GemRB.GetVar('Class') - 1, 3, GTV_INT) # Finds base class row id
		if ClassColumn < 1: ClassColumn = GemRB.GetVar('Class') - 1 # If 0 then already a base class so need actual row
		else: ClassColumn -= 1 # 'CLASS' column in classes.2da is out by 1 for some reason
		ClassColumn += 4 # There are 4 columns before the classes in skills.2da
		# At the moment only cleric kits get skill bonuses but their column names in skills.2da don't match up
		# to their kit names. All classes aren't covered in skills.2da either which is why I have to resort
		# to calculating the base class. This isn't ideal. Recommend a new 2DA be created with *all* classes
		# as rows and skills as columns. Something like SKILCLAS.2DA
		
		### Cleric kit hack:
		if GemRB.GetVar('Class') in range(27, 36):
			ClassColumn = GemRB.GetVar('Class') - 12
		
		RaceName = Tables[0].GetRowName(Tables[0].FindValue(3, GemRB.GetVar('Race')))
		SkillColumn = Tables[0].GetValue(RaceName, 'SKILL_COLUMN', GTV_INT) + 1
		Lookup = {'STR': 0, 'DEX': 1, 'CON': 2, 'INT': 3, 'WIS': 4, 'CHR': 5} # Probably a better way to do this
		for i in range(Tables[4].GetRowCount()):
			Abl = Tables[4].GetValue(i, 1, GTV_STR)
			Ranks = GemRB.GetVar('Skill ' + str(i))
			value = Ranks
			value += (GemRB.GetVar('Ability ' + str(Lookup[Abl])) / 2 - 5)
			value += Tables[5].GetValue(i, SkillColumn, GTV_INT)
			value += Tables[5].GetValue(i, ClassColumn, GTV_INT)
			
			untrained = Tables[5].GetValue(i, 3, GTV_INT)
			if not untrained and Ranks < 1:
				value = 0
			
			if value:
				strref = Tables[5].GetValue(i, 1)
				AddText(strref)
				strn = ': ' + str(value)
				if value != Ranks: strn += ' (' + str(Ranks) + ')'
				AddText(strn, 1)

		AddText('\n[color=FFFF00]' + GemRB.GetString(36310) + '[/color]', 1)
		
		for i in range(Tables[6].GetRowCount()):
			value = GemRB.GetVar('Feat ' + str(i))
			if value:
				strref = Tables[7].GetValue(i, 1)
				AddText(strref)
				multiple = Tables[6].GetValue(i, 0)
				if multiple != 0:
					AddText(': ' + str(value))
				AddText('\n')

		AddText('\n')
		import CommonTables
		import Spellbook
		MyChar = GemRB.GetVar ("Slot")
		BookTypes = { IE_IWD2_SPELL_BARD:39341, IE_IWD2_SPELL_CLERIC:11028, \
			IE_IWD2_SPELL_DRUID:39342, IE_IWD2_SPELL_PALADIN:39343, IE_IWD2_SPELL_RANGER:39344, \
			IE_IWD2_SPELL_SORCERER:39345, IE_IWD2_SPELL_WIZARD:11027, IE_IWD2_SPELL_DOMAIN:0 }
		for bt in BookTypes:
			KnownSpells = Spellbook.GetKnownSpells(MyChar, bt)
			if len(KnownSpells):
				BTName = BookTypes[bt]
				# individual domains, luckily in kit order
				if BTName == 0:
					KitIndex = GemRB.GetVar ("Class") - 1
					KitIndex -= CommonTables.Classes.FindValue ("CLASS", GemRB.GetVar ("BaseClass"))
					BTName = 39346 + KitIndex
				AddText ('\n[color=FFFF00]')
				AddText (BTName)
				AddText ('[/color]\n')
				for ks in KnownSpells:
					AddText (GemRB.GetString(ks['SpellName'])+"\n")
	
	# And we're done, w00t!
	CharGenWindow.Focus()
	return

def NextPress():
	if len(Steps) > GlobalStep - 1:
		GemRB.SetNextScript(Steps[GlobalStep - 1])
	else: #start the game
		import CharGen9
		CharGen9.NextPress()
	return

def CancelPress():
	#destroy the half generated character
	slot = GemRB.GetVar("Slot")
	GemRB.CreatePlayer("", slot|0x8000)
	if CharGenWindow:
		CharGenWindow.Close ()
	GemRB.SetNextScript('SPPartyFormation')
	return

def StartOver():
	StartOverWindow = GemRB.LoadWindow(53)
	
	def RestartGen():
		StartOverWindow.Close()
		CharGenWindow.Close()
		GemRB.SetNextScript('CharGen')
		return
	
	YesButton = StartOverWindow.GetControl(0)
	YesButton.SetText(13912)
	YesButton.OnPress (RestartGen)
	
	NoButton = StartOverWindow.GetControl(1)
	NoButton.SetText(13913)
	NoButton.OnPress (StartOverWindow.Close)
	
	TextAreaControl = StartOverWindow.GetControl(2)
	TextAreaControl.SetText(40275)
	return

def ImportPress():
	GemRB.SetToken('NextScript', 'CharGen')
	GemRB.SetNextScript('ImportFile')
	return

def BackPress():
	# Need to clear relevant variables
	if GlobalStep == 2: GemRB.SetVar('Gender', 0)
	elif GlobalStep == 3: GemRB.SetVar('Race', 0)
	elif GlobalStep == 4: GemRB.SetVar('Class', 0)
	elif GlobalStep == 5: GemRB.SetVar('Alignment', 0)
	elif GlobalStep == 6:
		for i in range(0, 7):
			GemRB.SetVar('Ability ' + str(i), 0)

	elif GlobalStep == 9: GemRB.SetToken('CHARNAME', '')
	
	ScrName = 'CharGen' + str(GlobalStep - 1)
	if GlobalStep == 2: ScrName = 'CharGen'
	GemRB.SetNextScript(ScrName)
	return
