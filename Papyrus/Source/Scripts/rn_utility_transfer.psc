Scriptname RN_Utility_Transfer extends Quest 

Import RN_Utility_Global
import JZCL_TCCHelperFunctions

RN_PatchAPI property API auto
 
RN_Utility_MCM property MCM auto

;;Alias to force the base item into.
referencealias property FoundAlias auto
message property TCC_TransferContainer auto
message property TCC_TransferComplete auto
message property TransferCompleteNoItems auto
message property TCC_RetrievalComplete auto

message property TransferComplete auto
message property TransferDisplayWait auto
message property TransferDisplayDone auto
message property DBM_SortError auto

objectreference property RN_Storage_Container auto
objectreference property RN_Excluded_Act auto
objectreference property DBM_AutoSortDropOff auto
objectreference property DBM_PrepStation auto
objectreference property PlayerRef auto

formlist property dbmMaster auto
formlist property dbmDisp auto
formlist property RN_ExcludedItems_Generic auto
formlist property DBM_ProtectedItems auto

globalvariable property DBM_SortWait auto
globalvariable property DBM_DisplayCount auto

int Transfered
int PlayerTransfered
int OldDisplayCount

Bool Property SilentTransfer auto hidden

Function RunCustomTransfer()
	
	DBM_SortWait.SetValue(1)
	Transfered = CustomTransfer(DBM_AutoSortDropOff, API.TokenRefList_NoShipment)
	
	DBM_SortWait.SetValue(0)
	DisplayFunc()
endFunction

Function RunAllTransfer()
	
	DBM_SortWait.SetValue(1)
	Transfered = AllTransfer(PlayerRef, DBM_AutoSortDropOff, API.TokenRefList)

	DBM_SortWait.SetValue(0)
	DisplayFunc()
endFunction

Function RunRelicTransfer()
	
	DBM_SortWait.SetValue(1)
	Transfered = RelicTransfer(RN_Storage_Container, DBM_AutoSortDropOff)

	DBM_SortWait.SetValue(0)
	DisplayFunc()
endFunction

Function DisplayFunc()
	
	TCCDebug.Log("Display - Started Displaying Tranfered Items...")
	
	if (Transfered > 0)
		Int Index = TransferComplete.Show(Transfered as Int)
		
		if (Index == 0)
		
			OldDisplayCount = DBM_DisplayCount.GetValue() as Int		
			DBM_PrepStation.Activate(DBM_AutoSortDropOff)
			
			If (!Utility.IsInMenuMode())
				TransferDisplayWait.Show()
			endIf
			
			Index = 0
			
			While DBM_SortWait.GetValue()
				Utility.Wait(1)
				Index += 1
				if (Index == 20)
					if (!Utility.IsInMenuMode())
						TransferDisplayWait.Show()
					endIf
					Index = 0
				endIF
			endWhile
			
			Transfered = (DBM_DisplayCount.GetValue() as Int - OldDisplayCount)
			
			TransferDisplayDone.Show(Transfered as Int)
		else
			Return
		endIf
	else
		TransferCompleteNoItems.Show(Transfered as Int)
	endIF
	
	TCCDebug.Log("Display - Finished Displaying Tranfered Items")
endFunction

Function TransferRelics(ObjectReference ref)
	
	Bool Transferable
	PlayerTransfered = 0
	
	if (ref) && (ref.GetBaseObject().GetType() == 28)
		if (!DBM_SortWait.GetValue())
			FoundAlias.ForceRefTo(ref)
			Int MenuButton = TCC_TransferContainer.Show()
			if (MenuButton == 0)	
				SilentTransfer = True
				DBM_SortWait.SetValue(1)
				Notify("The Curators Companion: Transfering Items...", MCM.ColourString)
				Int _Index = PlayerRef.GetNumItems()
				while _Index
					_Index -= 1		
					Form ItemRelic = PlayerRef.GetNthForm(_Index)
					if (MCM.Restricted) && (ref == RN_Storage_Container)
						Transferable = (dbmMaster.HasForm(ItemRelic)) && (!dbmDisp.HasForm(ItemRelic)) && (!Game.GetPlayer().IsEquipped(ItemRelic)) && (!Game.IsObjectFavorited(ItemRelic)) && (!RN_ExcludedItems_Generic.HasForm(ItemRelic)) && (!DBM_ProtectedItems.HasForm(ItemRelic))
					else
						Transferable = (dbmMaster.HasForm(ItemRelic)) && (!Game.GetPlayer().IsEquipped(ItemRelic)) && (!Game.IsObjectFavorited(ItemRelic)) && (!RN_ExcludedItems_Generic.HasForm(ItemRelic)) && (!DBM_ProtectedItems.HasForm(ItemRelic))
					endif
					
					if (Transferable)
						PlayerRef.RemoveItem(ItemRelic, PlayerRef.GetItemCount(ItemRelic), true, ref)
						PlayerTransfered += 1
					endIf
				endWhile
				SilentTransfer = False
				DBM_SortWait.SetValue(0)
				TCC_TransferComplete.show(PlayerTransfered)
				
			elseif (MenuButton == 1)
				DBM_SortWait.SetValue(1)
				Notify("The Curators Companion: Retrieving Items...", MCM.ColourString)
				Int _Index = ref.GetNumItems()
				while _Index
					_Index -= 1		
					Form ItemRelic = ref.GetNthForm(_Index)
					if (dbmMaster.HasForm(ItemRelic))
						ref.RemoveItem(ItemRelic, ref.GetItemCount(ItemRelic), true, PlayerRef)
						PlayerTransfered += 1
					endIf
				endWhile
				DBM_SortWait.SetValue(0)
				TCC_RetrievalComplete.show(PlayerTransfered)
			
			elseif (MenuButton == 2)
				FoundAlias.Clear()
				return
			endif
			
			FoundAlias.Clear()
		else
			DBM_SortError.Show()
		endIf
	else
		Notify("This spell can only be used on containers", MCM.ColourString)
	endif
endFunction

