scriptname RN_Utility_Script extends quest

;;--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
;;------------------------------------------------------------------------------ Script Properties -------------------------------------------------------------------------------------------------
;;--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

import AhzMoreHud
import AhzMoreHudIE
import RN_Utility_Global
import utility

RN_PatchAPI property API auto
RN_Utility_MCM property RN_MCM auto
RN_Utility_PropManager property Util auto
TCC_IconSetScript property IconScript auto

DBM_ReplicaHandler property ReplicaHandler auto

;;--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
;;----------------------------------------------------------------------------- General Properties -------------------------------------------------------------------------------------------------
;;--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

;;Message Properties
message property DBM_ScanMuseum_Message auto
message property DBM_ScanMuseum_Finished_Message auto
message property ModStartup auto
message property ModStartupDone auto
message property ModStartup_UpdatingLists auto
Message property ModConfigFinished auto

bool bSettingup
bool bScanning
bool bMoreHUDListsCreated
bool Maintaining

;;Do Not Fill this property.
objectreference property DBM_CloaksStorage auto

;;Player Ref 
objectreference property PlayerRef auto

;Relic Storage Spells
Book Property RN_RSC_SpellTome auto
Book Property RN_TransferContainer_SpellTome auto
Leveleditem Property LItemSpellTomes00AllSpells auto

;;--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
;;----------------------------------------------------------------------------- Formlist Properties ------------------------------------------------------------------------------------------------
;;--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

;;MoreHud
formlist property dbmNew auto
formlist property dbmDisp auto
formlist property dbmFound auto
formlist property dbmMaster auto

;;Replica Formlists
formlist property DBM_ReplicaBaseItems auto
formlist property DBM_ReplicaItems auto

;;Main Storage
formlist property _MuseumContainerList auto
formlist property _MuseumContainerList_WP auto

Formlist property _Armory_Formlist_Displays auto
Formlist property _Armory_Global_Total auto

Formlist property _Museum_Formlist_Merged auto
Formlist property _Museum_Global_Total auto
Formlist property _Museum_Global_Edits auto

;;--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
;;------------------------------------------------------------------------------ Global Variables --------------------------------------------------------------------------------------------------
;;--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

globalvariable property RN_moreHUD_Option auto
globalvariable property DBM_SortWait auto

globalvariable property RN_Setup_Start auto
globalvariable property RN_Setup_Finish auto

globalvariable property RN_Scan_Done auto
globalvariable property RN_Scan_Registered auto

bool property SetupDone auto hidden
bool property SetupDone2 auto hidden

;;-- Events ---------------------------------------
		
Event OnInit()

	GoToState("initialsetup")
	RegisterForSingleUpdate(0)
endEvent
	
;;-- Events ---------------------------------------

State initialsetup

	Event OnUpdate()
		
		float ftimeStart = Utility.GetCurrentRealTime()
		
		LItemSpellTomes00AllSpells.AddForm(RN_RSC_SpellTome, 1 , 1)
		LItemSpellTomes00AllSpells.AddForm(RN_TransferContainer_SpellTome, 1 , 1)
	
		if RN_Setup_Finish.GetValue()
			TCCDebug.Log("Utility - Setup already completed with " + RN_Setup_Finish.GetValue(), 0)	
		else
			
			DBM_SortWait.setvalue(1)
			RN_Setup_Start.setvalue(1)
			
			RN_MCM.Build_Arrays()
			
			while !SetupDone || !SetupDone2
				Wait(2)
			endWhile
			
			API.CheckPatches()
			ManageLists()
			CreateMoreHudLists()
			InitGlobals()			
			
			SendModEvent("FireScripts")
			
			RN_MCM.BuildPatchArray(true, true, false, false)	
			RN_MCM.AutoLoadConfig()
			
			RN_Setup_Start.setvalue(0)
			RN_Setup_Finish.setvalue(1)		
			DBM_SortWait.setvalue(0)				
			API.UnregisteredPatch = false	
			
			While IsInMenuMode()
				Wait(1)
			endWhile
		endIf

		TCCDebug.Log("Utility - Initial Mod Setup Finished", 0)
		
		float ftimeEnd = Utility.GetCurrentRealTime()
		TCCDebug.Log("Utility - Setup function completed in " + (ftimeEnd - ftimeStart) + " seconds.")
		
		ModConfigFinished.Show(API.SupportedModHandlers.Find(none))
		GoToState("Running")
	endevent
endstate

;;-- Functions ---------------------------------------

function ManageLists()

	_MuseumContainerList_WP.AddForm(PlayerRef)
		
	SendModEvent("Update_TokenArray", "Updating Token Array")
endfunction
				
;;-- Functions ---------------------------------------

function CreateMoreHudLists()		
	
	TCCDebug.Log("Utility - Updating moreHUD Lists...", 0)
	
	Int _Index = _MuseumContainerList.GetSize()	;;Check museum containers for displayed items.	
	While _Index 
		_Index -= 1
		ObjectReference _Container = _MuseumContainerList.GetAt(_Index) as ObjectReference
		if _Container
			TCCDebug.Log("Utility - Updating items in [" + _Container.GetBaseObject().GetName() + "]" + _Container, 0)
			Int _Index2 = _Container.GetNumItems()
			while _Index2
				_Index2 -= 1		
				Form _ItemRelic = _Container.GetNthForm(_Index2)
				if dbmNew.HasForm(_ItemRelic) && !dbmDisp.HasForm(_ItemRelic) || dbmFound.HasForm(_ItemRelic) && !dbmDisp.HasForm(_ItemRelic) 
					dbmNew.RemoveAddedForm(_ItemRelic)
					dbmFound.RemoveAddedForm(_ItemRelic)
					dbmDisp.AddForm(_ItemRelic)
					ProcessReplica(_ItemRelic)
				endIf
			endWhile
		endif
	endWhile
			
	_Index = API.TokenRefList.length ;; Check player and custom storage for found items.
	While _Index
		_Index -= 1
		ObjectReference _Container = API.TokenRefList[_Index] as ObjectReference		
		if _Container
			TCCDebug.Log("Utility - Updating items in [" + _Container.GetBaseObject().GetName() + "]" + _Container, 0)

			Int _Index2 = _Container.GetNumItems()
			While _Index2
				_Index2 -= 1
				Form _ItemRelic = _Container.GetNthForm(_Index2)
				if dbmNew.HasForm(_ItemRelic) && !dbmDisp.HasForm(_ItemRelic)
					dbmNew.RemoveAddedForm(_ItemRelic)
					dbmFound.AddForm(_ItemRelic)
				endIf
			endWhile
		endif
	endWhile			
	
	bMoreHUDListsCreated = true
	
	TCCDebug.Log("Utility - moreHUD Lists Updated", 0)
endFunction

function ProcessReplica(form _ItemRelic)

	if DBM_ReplicaBaseItems.HasForm(_ItemRelic)
		Form rReplica = (ReplicaHandler.GetReplica(_ItemRelic))
		if (!rReplica as Formlist)
			dbmNew.RemoveAddedForm(ReplicaHandler.GetReplica(_ItemRelic))
			dbmFound.RemoveAddedForm(ReplicaHandler.GetReplica(_ItemRelic))
			dbmDisp.AddForm(ReplicaHandler.GetReplica(_ItemRelic))
		endif
			
	elseif DBM_ReplicaItems.HasForm(_ItemRelic)	
		Form rOriginal = (ReplicaHandler.GetOriginal(_ItemRelic))
		if (!rOriginal as Formlist)
			dbmNew.RemoveAddedForm(ReplicaHandler.GetOriginal(_ItemRelic))
			dbmFound.RemoveAddedForm(ReplicaHandler.GetOriginal(_ItemRelic))
			dbmDisp.AddForm(ReplicaHandler.GetOriginal(_ItemRelic))
		endif
	endIf
endFunction

;;-- Events ---------------------------------------

function InitGlobals()
	
	Int _Index = 0
	Int _Length = _Armory_Global_Total.GetSize()
	While _Index < _Length
		Formlist _DisplayList = _Armory_Formlist_Displays.GetAt(_Index) as Formlist
		GlobalVariable _Total = _Armory_Global_Total.GetAt(_Index) as GlobalVariable	
		if _Index == 19 ;; Remove Thane Weapon Variants
			_Total.SetValue(_DisplayList.GetSize() - 6)
		else
			_Total.SetValue(_DisplayList.GetSize())
		endIf
		_Index += 1
	endWhile

	_Index = 0
	_Length = _Museum_Global_Total.GetSize()
	While _Index < _Length
		Formlist _DisplayList = _Museum_Formlist_Merged.GetAt(_Index) as Formlist
		GlobalVariable _Total = _Museum_Global_Total.GetAt(_Index) as GlobalVariable	
		GlobalVariable _Edit = _Museum_Global_Edits.GetAt(_Index) as GlobalVariable
		if _Index == 0
			_Total.SetValue((_DisplayList.GetSize() - _Edit.GetValue()) - 6)
		else
			_Total.SetValue(_DisplayList.GetSize() - _Edit.GetValue())
		endIf
		_Index += 1
	endWhile
	
	if (Game.GetModByName("LOTD_TCC_Cloaks.esp") != 255)
		DBM_CloaksStorage = Game.GetFormFromFile(2122, "DBM_CloaksofSkyrim_Patch.esp") as objectreference
		TCCDebug.Log("Utility - Found Cloaks Storage - " + DBM_CloaksStorage.GetBaseObject().GetName(), 0)
	endif
endfunction
	
		
Function Maintenance()
	
	if Maintaining
		TCCDebug.Log("Utility - Maintenance function already running - (exiting)", 0)
	else
		RegisterForSingleUpdate(1)
	endif
endFunction

;;-- Functions ---------------------------------------

Event onUpdate()
	
	float ftimeStart = Utility.GetCurrentRealTime()

	Maintaining = true

	RN_MCM.NotificationColour.GetBaseObject().SetName(RN_MCM.ColourString)
			
	if (RN_MCM.ShowStartup) 
		ModStartup.Show()
	endIf
	
	RN_Setup_Finish.setvalue(0)
	RN_Setup_Start.setvalue(1)
	DBM_SortWait.setvalue(1)
	
	if API.UnregisteredPatch
		ModStartup_UpdatingLists.Show()
		Wait(5)
		API.CheckPatches()
		API.UpdateCounts()
		InitGlobals()
		CreateMoreHudLists()
		RN_MCM.BuildPatchArray(true, true, true, false)
		API.UnregisteredPatch = False
		ModConfigFinished.Show(API.SupportedModHandlers.Find(none))
	endif
	
	ManageLists()
	IconScript.SetMainHud(RN_MCM.IndexmoreHUD)
	IconScript.SetInventoryHud(RN_MCM.IndexmoreHUDInventory)
	
	RN_Setup_Start.setvalue(0)
	RN_Setup_Finish.setvalue(1)			
	DBM_SortWait.setvalue(0)	
	
	SendModEvent("FireScripts")
	
	if RN_MCM.ShowStartup 
		ModStartupDone.Show()
	endIf
	Maintaining = false
	
	float ftimeEnd = Utility.GetCurrentRealTime()
	TCCDebug.Log("Util - Maintenance function completed in " + (ftimeEnd - ftimeStart) + " seconds.")
endEvent

;;-- Functions ---------------------------------------

function ScanMuseum()
	
	TCCDebug.Log("Utility - Scanning Museum...", 0)
	While IsInMenuMode()
		Wait(1)
	endWhile

	DBM_ScanMuseum_Message.Show()
	
	bScanning = True
	DBM_SortWait.setvalue(1)
	SendModEvent("TCCScan")
	FinishScan(5)
endFunction

;;-- Functions ---------------------------------------

function FinishScan(Int _Wait)
	
	Wait(_Wait)
	
	while bScanning	
		if RN_Scan_Done.GetValue() == RN_Scan_Registered.GetValue()			
			bScanning = False
			
			RN_Scan_Done.setvalue(0)
			DBM_SortWait.setvalue(0)
			RN_Scan_Registered.setvalue(0)
			While IsInMenuMode()
				Wait(1)
			endWhile
			
			DBM_ScanMuseum_Finished_Message.Show()
		endIf		
	endWhile
	
	TCCDebug.Log("Utility - Museum Scan Finished", 0)
endFunction

;;-- Functions ---------------------------------------

Function RebuildLists()

	TCCDebug.Log("Utility - Rebuild moreHUD Lists Request Received...", 0)
	
	Notify("The Curators Companion: Rebuilding moreHUD Lists...", RN_MCM.ColourString)
	Notify("The Curators Companion: Do not add / remove any items from inventory", RN_MCM.ColourString)
	
	DBM_SortWait.SetValue(1)
	
	dbmNew.revert()
	dbmFound.revert()
	dbmDisp.revert()	
	
	Int _Total = dbmMaster.GetSize()
	Int _Index = 0
	While _Index < _Total
		form _item = dbmMaster.GetAt(_Index) as form
		dbmNew.AddForm(_item)

		if (_Index % 500 == 0) 
			Notify("The Curators Companion: Rebuilding moreHUD lists... (" + _Index + " / " + _Total + ")", RN_MCM.ColourString)
		endIf
		_Index += 1
	endWhile
	
	CreateMoreHudLists()

	DBM_SortWait.SetValue(0)
	
	Notify("The Curators Companion: moreHUD Lists Rebuilt & Ready", RN_MCM.ColourString)
	TCCDebug.Log("Utility - Rebuild moreHUD Lists Request Completed", 0)
	TCCDebug.Log("Utility - dbmNew = " + dbmNew.GetSize() as Int, 0)
	TCCDebug.Log("Utility - dbmFound = " + dbmFound.GetSize() as Int, 0)
	TCCDebug.Log("Utility - dbmDisp = " + dbmDisp.GetSize() as Int, 0)
	TCCDebug.Log("Utility - dbmMaster = " + dbmMaster.GetSize() as Int, 0)
endFunction

;;-- Functions ---------------------------------------

Function ConfirmIcons()
	
	TCCDebug.Log("Utility - Checking moreHUD Lists for duplicate forms...", 0)
	
	Notify("The Curators Companion: Checking moreHUD Lists...", RN_MCM.ColourString)
	Notify("The Curators Companion: Do not add / remove any items from inventory", RN_MCM.ColourString)
	
	DBM_SortWait.SetValue(1)
	Int Index = 0
	While Index < dbmNew.GetSize()
		form _item = dbmNew.GetAt(Index) as form
		if (dbmDisp.Hasform(_item) || dbmFound.HasForm(_item))
			TCCDebug.Log("Utility - Found duplicate formID [" + _item.GetName() + "] " + _Item + " at position " + Index + "  while checking the dbmNew list", 2)
		endif

		if (!dbmMaster.HasForm(_item))
			TCCDebug.Log("Utility - Found erroneous formID [" + _item.GetName() + "] " + _Item + " at position " + Index + "  while checking the dbmNew list", 2)
		endif
			
		if (Index % 250 == 0) 
			Notify("The Curators Companion: Checking dbmNew moreHUD list (" + Index + " / " + dbmNew.GetSize() + ")", RN_MCM.ColourString)
		endIf	
		Index += 1
	endWhile
	
	Index = 0
	While Index < dbmFound.GetSize()
		form _item = dbmFound.GetAt(Index) as form
		if (dbmNew.Hasform(_item) || dbmDisp.HasForm(_item))
			TCCDebug.Log("Utility - Found duplicate formID [" + _item.GetName() + "] " + _Item + " at position " + Index + "  while checking the dbmFound list", 2)
		endif

		if (!dbmMaster.HasForm(_item))
			TCCDebug.Log("Utility - Found erroneous formID [" + _item.GetName() + "] " + _Item + " at position " + Index + "  while checking the dbmFound list", 2)
		endif
			
		if (Index % 250 == 0) 
			Notify("The Curators Companion: Checking dbmFound moreHUD list (" + Index + " / " + dbmFound.GetSize() + ")", RN_MCM.ColourString)
		endIf	
		Index += 1
	endWhile
	
	Index = 0
	While Index < dbmDisp.GetSize()
		form _item = dbmDisp.GetAt(Index) as form
		if (dbmNew.Hasform(_item) || dbmFound.HasForm(_item))
			TCCDebug.Log("Utility - Found duplicate formID [" + _item.GetName() + "] " + _Item + " at position " + Index + "  while checking the dbmDisp list", 2)
		endif

		if (!dbmMaster.HasForm(_item))
			TCCDebug.Log("Utility - Found erroneous formID [" + _item.GetName() + "] " + _Item + " at position " + Index + "  while checking the dbmDisp list", 2)
		endif
			
		if (Index % 250 == 0) 
			Notify("The Curators Companion: Checking dbmDisp moreHUD list (" + Index + " / " + dbmDisp.GetSize() + ")", RN_MCM.ColourString)
		endIf	
		Index += 1
	endWhile
	DBM_SortWait.SetValue(0)
	
	Notify("The Curators Companion: moreHUD Lists check complete", RN_MCM.ColourString)
endFunction

;;--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
;;--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
;;------------------------------------------------------------------------------ End of Main Script ------------------------------------------------------------------------------------------------
;;--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
;;--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------