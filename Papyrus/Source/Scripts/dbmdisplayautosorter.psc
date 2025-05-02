Scriptname DBMDisplayAutoSorter extends ObjectReference Conditional
{Master sorting script for Dragonborn Gallery v5.0 }

import JZCL_TCCHelperFunctions

DBM_MuseumAPI Property DBM_MuseumUtility Auto
;DBM_SKSESupport Property SKSE64 Auto
;DBM_SharedData Property Sharing Auto
DBM_ReplicaHandler Property DBM_Replicas Auto

Actor Property PlayerRef Auto

Message Property DBM_SortMenuMSG Auto
{Pop-up message to start sorting.}
Message Property DBM_SortStartMSG Auto
{Default = "All items being displayed, this will take a moment"}
Message Property DBM_SortUpdateMSG Auto
{Default = "Still sorting displays."}
Message Property DBM_SortCompleteMSG Auto
{Default = "Sort operation complete. Display Total: (DBM_DisplayMAX)"}
Message Property DBM_SortError Auto
{Default = "You must wait for the current sorting operation to complete before removing displays"}
Message Property DBM_SortMenuSettingsMSG Auto
{Sorting option toggles.}

GlobalVariable Property DBM_SortWait Auto
GlobalVariable Property DBM_DisplayCount Auto
GlobalVariable Property DBM_useSKSE Auto
GlobalVariable Property DBM_PrepstationOnlyReplicas Auto
GlobalVariable Property DBM_PrepstationPreferReplicas Auto
GlobalVariable Property DBM_PrepstationQuestItemsProtected Auto

Int iScriptVersion

Formlist Property DBM_ExcludeList Auto
Formlist Property DBM_ProtectedItems Auto

;;API Properties
Formlist[] Property RoomList Auto Hidden
String[] Property RoomNames Auto Hidden
Formlist[] Property SectionList Auto Hidden
String[] Property SectionNames Auto Hidden
Formlist[] Property SectionItems Auto Hidden
Formlist[] Property SectionItemsAlt Auto Hidden
Formlist[] Property SectionList2 Auto Hidden
String[] Property SectionNames2 Auto Hidden
Formlist[] Property SectionItems2 Auto Hidden
Formlist[] Property SectionItemsAlt2 Auto Hidden
; 5.6.0 Update
Formlist[] Property SectionList3 Auto Hidden
String[] Property SectionNames3 Auto Hidden
Formlist[] Property SectionItems3 Auto Hidden
Formlist[] Property SectionItemsAlt3 Auto Hidden
Formlist[] Property SectionList4 Auto Hidden
String[] Property SectionNames4 Auto Hidden
Formlist[] Property SectionItems4 Auto Hidden
Formlist[] Property SectionItemsAlt4 Auto Hidden
FormList Property DBM_SortingArmory Auto
FormList Property DBM_SortingBooks Auto
FormList Property DBM_SortingJewelry Auto

;;Display Object containers
ObjectReference Property DISPLAYCHEST Auto
ObjectReference Property BookCabinet Auto
ObjectReference Property ArmoryCabinet Auto
ObjectReference Property JewleryCabinet Auto

Bool useSKSE
Bool bPreferReplicas
Bool bOnlyReplicas
Bool bQuestItemsProtected
Bool bPlayerActivation

;Desired features (Count of new displays), Replica only, prefer replicas, count monitoring.

Int Function GetScriptVersion()
	Return 1
EndFunction

Function CheckForUpdate(int iSavedVersion)
	if iSavedVersion < GetScriptVersion()
		;Do some update stuff.
	endif
EndFunction

Function UpdateLists()
	{Grab the lists from the API}
	RoomList = DBM_MuseumUtility.RoomLists
	RoomNames = DBM_MuseumUtility.RoomNames
	SectionList = DBM_MuseumUtility.SectionDisplayLists
	SectionNames = DBM_MuseumUtility.SectionNames
	SectionItems = DBM_MuseumUtility.SectionDisplayItems
	SectionItemsAlt = DBM_MuseumUtility.SectionDisplayItemsAlt
	SectionList2 = DBM_MuseumUtility.SectionDisplayLists2
	SectionNames2 = DBM_MuseumUtility.SectionNames2
	SectionItems2 = DBM_MuseumUtility.SectionDisplayItems2
	SectionItemsAlt2 = DBM_MuseumUtility.SectionDisplayItemsAlt2
	; 5.6.0 Update
	SectionList3 = DBM_MuseumUtility.SectionDisplayLists3
	SectionNames3 = DBM_MuseumUtility.SectionNames3
	SectionItems3 = DBM_MuseumUtility.SectionDisplayItems3
	SectionItemsAlt3 = DBM_MuseumUtility.SectionDisplayItemsAlt3
	SectionList4 = DBM_MuseumUtility.SectionDisplayLists4
	SectionNames4 = DBM_MuseumUtility.SectionNames4
	SectionItems4 = DBM_MuseumUtility.SectionDisplayItems4
	SectionItemsAlt4 = DBM_MuseumUtility.SectionDisplayItemsAlt4
	bPreferReplicas = DBM_PrepstationPreferReplicas.GetValue()
	bOnlyReplicas = DBM_PrepstationOnlyReplicas.GetValue()
	bQuestItemsProtected = DBM_PrepstationQuestItemsProtected.GetValue()
EndFunction

Auto State Ready
	Event OnActivate(ObjectReference akActionRef)
		CheckForUpdate(iScriptVersion) ;See if anything has changed since last load.
		UpdateLists() ;Grab the listings from the Museum API
		
		bPlayerActivation = False
		useSKSE = DBM_useSKSE.GetValue() ;1 = True, 0 = False
		
		if akActionRef == PlayerRef ;Ignore anyone who isn't the player.
			bPlayerActivation = True
		endif
		
		if DBM_SortWait.GetValue() == 1 ;Sort operation is running already.
			if bPlayerActivation
				DBM_SortError.Show()
				DBMDebug.Log(Self,"Sort operation cancelled as DBM_SortWait is 1.")
			endif
			Return
		endif
		
		Int iSortMenu
		if bPlayerActivation
			iSortMenu = DBM_SortMenuMSG.Show() ;Would you like to sort all items
		else
			iSortMenu = 0 ;Automatically sort if the player did not activate the station.
		endif
		
		if iSortMenu == 0 ;Yes
			if (useSKSE)
				DBMDebug.SendSortingStarted(Self)
			endif
			DBMDebug.Log(Self, "Starting Auto Sort operation with settings: PreferReplicas="+bPreferReplicas+", OnlyReplicas="+bOnlyReplicas+", QuestItemsProtected="+bQuestItemsProtected)
			DBM_SortWait.SetValue(1)
			GoToState("Busy")
			if (bPlayerActivation)
				DBM_SortStartMSG.Show()
			endif

			int iNewDisplays = 0
			AddSearchSection(SectionList, SectionNames, SectionItems, SectionItemsAlt)
			AddSearchSection(SectionList2, SectionNames2, SectionItems2, SectionItemsAlt2)
			AddSearchSection(SectionList3, SectionNames3, SectionItems3, SectionItemsAlt3)
			AddSearchSection(SectionList4, SectionNames4, SectionItems4, SectionItemsAlt4)
			
			If (!DBMSectionSearch(RoomList, RoomNames, akActionRef))
				Debug.Messagebox("Critical Autosorter error, operation aborted.")
				; Was not resetting the state on a catastrophic failure. 
				DBM_SortWait.SetValue(0)
				GoToState("Ready")
				Return
			EndIf

			While (NextSectionSearchResult())
				FormList[] aSectionFormLists = GetSectionSearchFormList()
				int iItemTotal = GetSectionSearchItemTotal()

				int iListNewDisplays = (SortDisplays(aSectionFormLists[0], aSectionFormLists[1], aSectionFormLists[2], iItemTotal, akActionRef))
				; If we added any new displays, send a mod event. 
				if (iListNewDisplays && useSKSE)
					DBMDebug.SendDisplayListUpdated(Self, aSectionFormLists[0], aSectionFormLists[1], aSectionFormLists[2])
				endif
				iNewDisplays += iListNewDisplays
			EndWhile
			ClearSectionSearch()


			DBM_SortWait.SetValue(0)
			GoToState("Ready")
			if (bPlayerActivation)
				DBM_SortCompleteMSG.Show(iNewDisplays, DBM_DisplayCount.GetValue())
			endif
			if (useSKSE)
				DBMDebug.SendSortingComplete(Self, DBM_DisplayCount.GetValue() as Int)
			endif
			DBMDebug.Log(Self, "Sort operation complete. Added " + iNewDisplays + " new displays. Total displays: " + DBM_DisplayCount.GetValue() as Int)
			;Sort operation done.
			
		elseif iSortMenu == 1 ;No
			Return
		elseif iSortMenu == 2 ;Settings
			int iConfig = DBM_SortMenuSettingsMSG.Show()
			if iConfig == 0
				DBM_PrepstationPreferReplicas.SetValue(0)
			elseif iConfig == 1
				DBM_PrepstationPreferReplicas.SetValue(1)
			elseif iConfig == 2
				DBM_PrepstationOnlyReplicas.SetValue(0)
			elseif iConfig == 3
				DBM_PrepstationOnlyReplicas.SetValue(1)
			elseif iConfig == 4
				DBM_PrepstationQuestItemsProtected.SetValue(0)
			elseif iConfig == 5
				DBM_PrepstationQuestItemsProtected.SetValue(1)
			endif
		endif
		
	EndEvent
EndState

State Busy
	;I'm sorting!
EndState

Int Function SortDisplays(Formlist flSection, Formlist flItems,Formlist flItemsAlt = None, Int iItemTotal, ObjectReference akActionRef)
	if (bPlayerActivation)
		;Post a notification so the player knows we're sorting.
		DBM_SortUpdateMSG.Show()
	endif
	
	;Select the correct container for the items. 
	ObjectReference oCont = DISPLAYCHEST ;Defined as default option. Assign non-default below.
	if DBM_SortingBooks.HasForm(flSection)
		oCont = BookCabinet
	elseif DBM_SortingArmory.HasForm(flSection)
		oCont = ArmoryCabinet
	elseif DBM_SortingJewelry.HasForm(flSection)
		oCont = JewleryCabinet
	endif
	
	int iNewDisplays = SortDisplays_SKSE(flSection, flItems, iItemTotal, oCont, akActionRef, bPreferReplicas, bOnlyReplicas, bQuestItemsProtected, useSKSE)


;	if Sharing.IsSharingOn
;		Sharing.UpdateSingleDisplayList(flSection)
;    endif
	
	Return iNewDisplays
EndFunction

Int Function CheckDisplay(ObjectReference Disp, Form Item, Form Replica = None, ObjectReference oCont, ObjectReference akActionRef)
	;DBMDebug.Log(Self, "Checking Item:"+Item.GetName()+Item+", Replica:"+Replica.GetName()+Replica+", Display:"+Disp)
	
	if !Item || !Disp
		DBMDebug.Log(Self, "Invalid item " + Item + " for display "+Disp)
		Return 0
	endif
	
	if bPreferReplicas && Replica ;if 'prefer replicas' and item player has replica, turn "item" into replica, else clear
		if akActionRef.GetItemCount(Replica)
			Item = Replica
		endif
	endif
			
	if bOnlyReplicas && Replica ;if 'only replicas' and item has a valid replica, turn "item" into replica.
		Item = Replica
	endif
	
	bool bIgnoreItem = False
	if bQuestItemsProtected && DBM_ProtectedItems.HasForm(Item) && akActionRef.GetItemCount(Item) ;if 'quest items protected' clear item. 
			DBMDebug.Log(Self, Item.GetName()+Item+" is a protected quest item and will not display automatically.")
			bIgnoreItem = True
	endif
	
			
	if (bOnlyReplicas || bPreferReplicas) && Replica
		if Replica as Formlist
			Formlist subList = Replica as FormList
			int iIndex2 = subList.GetSize()
			while iIndex2
				iIndex2 -= 1
				Replica = subList.GetAt(iIndex2)

				if Disp.IsDisabled() && (akActionRef.GetItemCount(Replica)) && !oCont.GetItemCount(Replica) && (!akActionRef as Actor || !(akActionRef as Actor).IsEquipped(Replica)) && !DBM_ExcludeList.HasForm(Replica) && (!useSKSE || !Game.IsObjectFavorited(Replica))
					akActionRef.RemoveItem(Replica, 1, true, oCont)
					DBM_DisplayCount.Mod(1)
					Disp.Enable()
					DBMDebug.Log(Self, "Displayed replica "+Replica.GetName() + Replica + " for " + Disp)
					if (useSKSE)
						DBMDebug.SendDisplayEvent(Self, Disp, Replica, true)
					endif
					Return 1
				endif
			endwhile
					
		else
			if Disp.IsDisabled() && (akActionRef.GetItemCount(Replica)) && !oCont.GetItemCount(Replica) && (!akActionRef as Actor || !(akActionRef as Actor).IsEquipped(Replica)) && !DBM_ExcludeList.HasForm(Replica) && (!useSKSE || !Game.IsObjectFavorited(Replica))
				akActionRef.RemoveItem(Replica, 1, true, oCont)
				DBM_DisplayCount.Mod(1)
				Disp.Enable()
				DBMDebug.Log(Self, "Displayed replica "+Replica.GetName() + Replica + " for " + Disp)
				if (useSKSE)
					DBMDebug.SendDisplayEvent(Self, Disp, Replica, true)
				endif
				Return 1
			endif		
		endif
	endif
	
	if !bIgnoreItem && !bOnlyReplicas && Item
		if Item as Formlist
			Formlist subList = Item as FormList
			int iIndex2 = subList.GetSize()
			while iIndex2
				iIndex2 -= 1
				Item = subList.GetAt(iIndex2)

				if Disp.IsDisabled() && (akActionRef.GetItemCount(Item)) && !oCont.GetItemCount(Item) && (!akActionRef as Actor || !(akActionRef as Actor).IsEquipped(Item)) && !DBM_ExcludeList.HasForm(Item) && (!useSKSE || !Game.IsObjectFavorited(Item))
					akActionRef.RemoveItem(Item, 1, true, oCont)
					DBM_Replicas.RemoveRadiantForm(Item)
					DBM_DisplayCount.Mod(1)
					Disp.Enable()
					DBMDebug.Log(Self, "Displayed item "+Item.GetName() + Item + " for " + Disp)
					if (useSKSE)
						DBMDebug.SendDisplayEvent(Self, Disp, Item, true)
					endif
					Return 1
				endif
			endwhile
					
		else
			if Disp.IsDisabled() && (akActionRef.GetItemCount(Item)) && !oCont.GetItemCount(Item) && (!(akActionRef as Actor) || !(akActionRef as Actor).IsEquipped(Item)) && !DBM_ExcludeList.HasForm(Item) && (!useSKSE || !Game.IsObjectFavorited(Item))
				akActionRef.RemoveItem(Item, 1, true, oCont)
				DBM_Replicas.RemoveRadiantForm(Item)
				DBM_DisplayCount.Mod(1)
				Disp.Enable()
				DBMDebug.Log(Self, "Displayed item "+Item.GetName() + Item + " for " + Disp)
				if (useSKSE)
					DBMDebug.SendDisplayEvent(Self, Disp, Item, true)
				endif
				Return 1
			endif		
		endif
	endif
	;DBMDebug.Log(Self, "Did not display "+Item.GetName()+Item)
	Return 0
EndFunction
