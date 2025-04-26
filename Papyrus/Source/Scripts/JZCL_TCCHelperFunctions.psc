Scriptname JZCL_TCCHelperFunctions Hidden

; search in FormList and direct sub FormList, if found return index, otherwise return -1
int Function FormListLevel2Search(Form akForm, FormList akList) global native

; add item in sourceList to targetList
Function FormListAdd(FormList targetList, FormList sourceList) global native

; remove item in sourceList from targetList
Function FormListSub(FormList targetList, FormList sourceList) global native

Function ReplicaAndVariantsAddHandler(Form akForm) global native

Function ReplicaAndVariantsRemoveHandler(Form akForm, ObjectReference[] akTokenRefList, ObjectReference akContainer) global native

Function RepopulateDbmNew() global native

Function updateMoreHUDLists(ObjectReference[] akTokenRefList) global native

int Function CustomTransfer(ObjectReference aDBM_AutoSortDropOff, ObjectReference[] aTokenRefList_NoShipment) global native

int Function AllTransfer(ObjectReference aPlayerRef, ObjectReference aDBM_AutoSortDropOff, ObjectReference[] aTokenRefList) global native

int Function RelicTransfer(ObjectReference aRN_Storage_Container, ObjectReference aDBM_AutoSortDropOff) global native

Function AddSearchSection(FormList[] SectionList, String[] SectionNames, FormList[] SectionItems, FormList[] SectionItemsAlt) global native

Bool Function DBMSectionSearch(FormList[] RoomList, String[] RoomNames, ObjectReference akActionRef) global native

Bool Function NextSectionSearchResult() global native

; return [1]flSection, [2]flItems, [3]flItemsAlt
FormList[] Function GetSectionSearchFormList() global native

int Function GetSectionSearchItemTotal() global native

Function ClearSectionSearch() global native
