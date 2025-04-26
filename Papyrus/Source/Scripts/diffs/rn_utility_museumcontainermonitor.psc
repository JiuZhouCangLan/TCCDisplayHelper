ScriptName RN_Utility_MuseumContainerMonitor extends ObjectReference

RN_PatchAPI property API auto
RN_Utility_MCM property RN_MCM auto
RN_Utility_Script property RN_Utility auto

DBM_ReplicaHandler property ReplicaHandler auto

Formlist property DBM_ReplicaBaseItems auto
formlist property DBM_ReplicaItems auto

formlist property dbmNew auto
formlist property dbmFound auto
formlist property dbmDisp auto

objectreference property DISPLAYCHEST auto

;-- Events --------------------------------

Event OnItemAdded(Form akBaseItem, int aiItemCount, ObjectReference akItemReference, ObjectReference akSourceContainer)

	if (dbmNew.HasForm(akBaseItem)) || (dbmFound.HasForm(akBaseItem))
		updateFormlists(akBaseitem, true)
	endIf
endEvent			

;-- Events --------------------------------

Event OnItemRemoved(Form akBaseItem, int aiItemCount, ObjectReference akItemReference, ObjectReference akDestContainer)

	if (akDestContainer == DISPLAYCHEST) || ((RN_Utility.DBM_CloaksStorage) && (akDestContainer == RN_Utility.DBM_CloaksStorage))
		;;Do Nothinhg
	else
		if (self.GetItemCount(akBaseItem) == 0)
			if (dbmDisp.HasForm(akBaseItem) )
				updateFormlists(akBaseitem, false, akDestContainer)
			endIf
		endIf	
	endIf			
endEvent

;-- Events --------------------------------

function updateFormlists(form akBaseItem, bool added, ObjectReference akContainer = none)

	if (added)
		dbmDisp.AddForm(akBaseItem)
		dbmFound.RemoveAddedForm(akBaseItem)
		dbmNew.RemoveAddedForm(akBaseItem)		
	else
		dbmDisp.RemoveAddedForm(akBaseItem)
		if (API.TokenRefList.Find(akContainer) == -1)
			dbmNew.AddForm(akBaseItem)
		else
			dbmFound.AddForm(akBaseItem)
		endIf
	endIf
	
	ProcessReplica(akBaseItem, added, akContainer)
endFunction

function ProcessReplica(form akBaseItem, bool added, ObjectReference akContainer = none)
		
	if (added)
		if (DBM_ReplicaBaseItems.HasForm(akBaseitem))
			Form rReplica = (ReplicaHandler.GetReplica(akBaseItem))	
			if (!rReplica as Formlist)
				dbmDisp.AddForm(rReplica)
				dbmFound.RemoveAddedForm(rReplica)
				dbmNew.RemoveAddedForm(rReplica)
			endIf
			
		elseif (DBM_ReplicaItems.HasForm(akBaseitem))
			Form rOriginal = (ReplicaHandler.GetOriginal(akBaseItem))
			if (!rOriginal as Formlist)
				dbmDisp.AddForm(rOriginal)
				dbmFound.RemoveAddedForm(rOriginal)
				dbmNew.RemoveAddedForm(rOriginal)
			endif
		endIf
		
	else
		if (DBM_ReplicaBaseItems.HasForm(akBaseitem))	
			Form rReplica = (ReplicaHandler.GetReplica(akBaseItem))	
			if (!rReplica as Formlist)
				dbmDisp.RemoveAddedForm(rReplica)
				if (API.TokenRefList.Find(akContainer) == -1)
					dbmNew.AddForm(rReplica)
				else
					dbmFound.AddForm(rReplica)
				endIf
			endif
			
		elseif (DBM_ReplicaItems.HasForm(akBaseitem))
			Form rOriginal = (ReplicaHandler.GetOriginal(akBaseItem))
			if (!rOriginal as Formlist)
				dbmDisp.RemoveAddedForm(rOriginal)
				if (API.TokenRefList.Find(akContainer) == -1)
					dbmNew.AddForm(rOriginal)
				else
					dbmFound.AddForm(rOriginal)
				endIf	
			endif
		endIf	
	endIf		
endFunction


