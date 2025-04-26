#include "TransferHandler.h"
#include "DisplayHandler.h"

#define GET_FORMLIST_LOG(variable, editorID) GET_VARIANT(RE::BGSListForm, variable, editorID, "{} is invalid, {}:TransferHandler will not work")

namespace Papyrus::Functions::TransferHandler
{
	constexpr auto DBMMASTER = "dbmMaster";
	constexpr auto DBMDISP = "dbmDisp";
	constexpr auto RN_EXCLUDEDITEMS_GENERIC = "RN_ExcludedItems_Generic";
	constexpr auto DBM_PROTECTEDITEMS = "DBM_ProtectedItems";

	static RE::BGSListForm* dbmMaster = nullptr;
	static RE::BGSListForm* dbmDisp = nullptr;
	static RE::BGSListForm* RN_ExcludedItems_Generic = nullptr;
	static RE::BGSListForm* DBM_ProtectedItems = nullptr;

	static bool init()
	{
		using namespace RE;

		static bool           hasError = false;
		static std::once_flag callFlag;
		std::call_once(callFlag, []() {
			GET_FORMLIST_LOG(dbmMaster, DBMMASTER);
			GET_FORMLIST_LOG(dbmDisp, DBMDISP);
			GET_FORMLIST_LOG(RN_ExcludedItems_Generic, RN_EXCLUDEDITEMS_GENERIC);
			GET_FORMLIST_LOG(DBM_ProtectedItems, DBM_PROTECTEDITEMS);
		});

		return !hasError;
	}

	bool IsEquipped(RE::Actor* actor, RE::TESForm* object)
	{
		if (actor == nullptr || object == nullptr) {
			return false;
		}

		if (actor->GetEquippedObject(true) == object || actor->GetEquippedObject(false) == object) {
			return true;
		}

		for (int i = 0; i < 32; i++) {
			auto* armor = actor->GetInventoryChanges()->GetArmorInSlot(i + 30);
			if (armor == object) {
				return true;
			}
		}

		return false;
	}

	int CustomTransfer(STATIC_ARGS, RE::TESObjectREFR* DBM_AutoSortDropOff, std::vector<RE::TESObjectREFR*> TokenRefList_NoShipment)
	{
		if (!init()) {
			return 0;
		}
		logger::debug("{} called", __FUNCTION__);

		RE::DebugNotification("The Curators Companion: Transferring Items...", 0, false);
		int transfered = 0;
		for (auto& container : TokenRefList_NoShipment) {
			if (container == nullptr) {
				continue;
			}
			auto*      actor = container->As<RE::Actor>();
			const auto inv = container->GetInventory();
			for (const auto& [item, data] : inv) {
				const auto& [numItem, entry] = data;
				auto* itemRelic = entry->GetObject()->As<RE::TESForm>();
				if (dbmMaster->HasForm(itemRelic) && !dbmDisp->HasForm(itemRelic)) {
					bool transferable = false;
					if (actor == nullptr) {
						transferable = !RN_ExcludedItems_Generic->HasForm(itemRelic) && !DBM_ProtectedItems->HasForm(itemRelic);
					} else {
						transferable = !IsEquipped(actor, itemRelic) && !entry->IsFavorited() && !RN_ExcludedItems_Generic->HasForm(itemRelic) && !DBM_ProtectedItems->HasForm(itemRelic);
					}

					if (transferable) {
						container->RemoveItem(item, 1, RE::ITEM_REMOVE_REASON::kStoreInContainer, nullptr, DBM_AutoSortDropOff);
						transfered += 1;
					}
				}
			}
		}
		return transfered;
	}

	int AllTransfer(STATIC_ARGS, RE::TESObjectREFR* PlayerRef, RE::TESObjectREFR* DBM_AutoSortDropOff, std::vector<RE::TESObjectREFR*> TokenRefList)
	{
		if (!init()) {
			return 0;
		}
		logger::debug("{} called", __FUNCTION__);

		RE::DebugNotification("The Curators Companion: Transferring Items...", 0, false);
		int transfered = 0;
		for (auto& container : TokenRefList) {
			if (container == nullptr) {
				continue;
			}
			auto*      actor = container->As<RE::Actor>();
			const auto inv = container->GetInventory();
			for (const auto& [item, data] : inv) {
				const auto& [numItem, entry] = data;
				auto* itemRelic = entry->GetObject()->As<RE::TESForm>();
				if (dbmMaster->HasForm(itemRelic) && !dbmDisp->HasForm(itemRelic)) {
					bool transferable = false;
					if (actor == nullptr) {
						transferable = !IsEquipped(PlayerRef->As<RE::Actor>(), itemRelic) && !entry->IsFavorited() && !RN_ExcludedItems_Generic->HasForm(itemRelic) && !DBM_ProtectedItems->HasForm(itemRelic);

					} else {
						transferable = !IsEquipped(actor, itemRelic) && !entry->IsFavorited() && !RN_ExcludedItems_Generic->HasForm(itemRelic) && !DBM_ProtectedItems->HasForm(itemRelic);
					}

					if (transferable) {
						container->RemoveItem(item, 1, RE::ITEM_REMOVE_REASON::kStoreInContainer, nullptr, DBM_AutoSortDropOff);
						transfered += 1;
					}
				}
			}
		}

		return transfered;
	}

	int RelicTransfer(STATIC_ARGS, RE::TESObjectREFR* RN_Storage_Container, RE::TESObjectREFR* DBM_AutoSortDropOff)
	{
		if (!init()) {
			return 0;
		}
		logger::debug("{} called", __FUNCTION__);

		RE::DebugNotification("The Curators Companion: Transferring Items...", 0, false);
		int        transfered = 0;
		const auto inv = RN_Storage_Container->GetInventory();
		for (const auto& [item, data] : inv) {
			const auto& [numItem, entry] = data;
			auto* itemRelic = entry->GetObject()->As<RE::TESForm>();
			if (dbmMaster->HasForm(itemRelic) && !dbmDisp->HasForm(itemRelic)) {
				if (!RN_ExcludedItems_Generic->HasForm(itemRelic) && !DBM_ProtectedItems->HasForm(itemRelic)) {
					RN_Storage_Container->RemoveItem(item, 1, RE::ITEM_REMOVE_REASON::kStoreInContainer, nullptr, DBM_AutoSortDropOff);
					transfered += 1;
				}
			}
		}

		return transfered;
	}

	struct SectionInfo
	{
		std::vector<RE::BGSListForm*> SectionList;
		std::vector<std::string>      SectionNames;
		std::vector<RE::BGSListForm*> SectionItems;
		std::vector<RE::BGSListForm*> SectionItemsAlt;
	};
	static std::vector<SectionInfo> SearchSections;

	void AddSearchSection(STATIC_ARGS,
		std::vector<RE::BGSListForm*> SectionList,
		std::vector<std::string>      SectionNames,
		std::vector<RE::BGSListForm*> SectionItems,
		std::vector<RE::BGSListForm*> SectionItemsAlt)
	{
		logger::debug("{} called", __FUNCTION__);
		SearchSections.emplace_back(SectionInfo{ SectionList, SectionNames, SectionItems, SectionItemsAlt });
	}

	struct ResultItem
	{
		RE::BGSListForm* flSection = nullptr;
		RE::BGSListForm* flItems = nullptr;
		RE::BGSListForm* flItemsAlt = nullptr;
		int              iItemTotal = 0;
	};
	static std::vector<ResultItem> searchResult;
	static int                     resultAccessIndex = -1;

	int getItemCount(const RE::TESObjectREFR::InventoryItemMap& inventory, RE::BGSListForm* formlist)
	{
		using namespace clib_util::editorID;
		int total = 0;
		for (const auto& [item, data] : inventory) {
			const auto& [count, entry] = data;
			if (formlist->HasForm(item)) {
				total += count;
			}
		}
		return total;
	}

	bool DBMSectionSearch(STATIC_ARGS, std::vector<RE::BGSListForm*> RoomList, std::vector<std::string> RoomNames, RE::TESObjectREFR* akActionRef)
	{
		/*
			int iRoomTotal = RoomList.Length
			int iCurRoom; Currently processing room.
			int iNewDisplays = 0 ;Recorded new displays.
			
			while iCurRoom < iRoomTotal
				Formlist flRoom = RoomList[iCurRoom] as Formlist
				String sRoomName = RoomNames[iCurRoom]
				;DBMDebug.Log(Self, "Auto sorting room: "+sRoomName+iCurRoom)
				if flRoom ;check if we have a valid room. 
					int iSectionTotal = flRoom.GetSize()
					int iCurSection = 0
					
					while iCurSection < iSectionTotal
						Formlist flSection = flRoom.GetAt(iCurSection) as FormList ;Get the section in this room, containing references. 
					
						;; Set the main arrays but change them later if required. 5.1.0+
						FormList[] DisplayList = SectionList
						String[] NameList = SectionNames
						FormList[] ItemList = SectionItems
						FormList[] AltItemList = SectionItemsAlt
						Int iSectionIndex = SectionList.Find(flSection)

						; 5.6.0 include lookup for all 4 arrays!
						if (SectionList.Find(flSection) != -1)
							DisplayList = SectionList
							NameList = SectionNames
							ItemList = SectionItems
							AltItemList = SectionItemsAlt
							iSectionIndex = SectionList.Find(flSection)
						elseif (SectionList2.Find(flSection) != -1)
							DisplayList = SectionList2
							NameList = SectionNames2
							ItemList = SectionItems2
							AltItemList = SectionItemsAlt2
							iSectionIndex = SectionList2.Find(flSection)
						elseif (SectionList3.Find(flSection) != -1)
							DisplayList = SectionList3
							NameList = SectionNames3
							ItemList = SectionItems3
							AltItemList = SectionItemsAlt3
							iSectionIndex = SectionList3.Find(flSection)
						elseif (SectionList3.Find(flSection) != -1)
							DisplayList = SectionList4
							NameList = SectionNames4
							ItemList = SectionItems4
							AltItemList = SectionItemsAlt4
							iSectionIndex = SectionList4.Find(flSection)
						else
							;; List isn't in either array. Critical error.
							DBMDebug.Log(Self, "Could not find "+flSection+" in the main or reserve arrays. Aborting sorter.", 2)
							Debug.Trace("LOTD Sorter - Could not find "+flSection+" in the main or reserve arrays. Aborting sorter.", 2)
							Debug.Messagebox("Critical Autosorter error, operation aborted.")
							; Was not resetting the state on a catastrophic failure. 
							DBM_SortWait.SetValue(0)
							GoToState("Ready")
							Return
						endif
						
						String sSectionName = NameList[iSectionIndex] ;Get the section name.
						Formlist flItems = ItemList[iSectionIndex] as FormList ;Get the item list
						Formlist flItemsAlt = AltItemList[iSectionIndex] as FormList ;Get the replica and alt item list
						
						if akActionRef.GetItemCount(flItems) || (!flItemsAlt ||akActionRef.GetItemCount(flItemsAlt))
							int iItemTotal = akActionRef.GetItemCount(flItems)
							if flItemsAlt
								;DBMDebug.Log(Self,"Adding "+ akActionRef.GetItemCount(flItemsAlt) +" to current iItemTotal of "+iItemTotal)
								iItemTotal += akActionRef.GetItemCount(flItemsAlt) ;Add alt items if applicable.
							endif
							if iItemTotal
								DBMDebug.Log(Self, "Auto sorting "+sSectionName+" in "+sRoomName+" for a total item count of "+iItemTotal)
								int iListNewDisplays = (SortDisplays(flSection, flItems,flItemsAlt, iItemTotal, akActionRef))
								; If we added any new displays, send a mod event. 
								if (iListNewDisplays && useSKSE)
									DBMDebug.SendDisplayListUpdated(Self, flSection, flItems, flItemsAlt)
								endif
								iNewDisplays += iListNewDisplays
							endif
						endif
						
						iCurSection += 1
					endwhile
				endif
				
				iCurRoom += 1
				
			endwhile
	*/

		logger::debug("{} called", __FUNCTION__);
		using namespace clib_util::editorID;

		for (size_t roomIndex = 0, roomCount = RoomList.size(); roomIndex < roomCount; ++roomIndex) {
			auto flRoom = RoomList.at(roomIndex);
			auto sRoomName = RoomNames[roomIndex];
			if (flRoom == nullptr) {
				continue;
			}
			for (auto& section : flRoom->forms) {
				if (section == nullptr) {
					continue;
				}
				auto                          flSection = section->As<RE::BGSListForm>();
				std::vector<RE::BGSListForm*> DisplayList;
				std::vector<std::string>      NameList;
				std::vector<RE::BGSListForm*> ItemList;
				std::vector<RE::BGSListForm*> AltItemList;
				int                           iSectionIndex = -1;

				for (auto& sectionInfo : SearchSections) {
					auto sectionIterator = std::find(sectionInfo.SectionList.begin(), sectionInfo.SectionList.end(), flSection);
					if (sectionIterator != sectionInfo.SectionList.end()) {
						DisplayList = sectionInfo.SectionItems;
						NameList = sectionInfo.SectionNames;
						ItemList = sectionInfo.SectionItems;
						AltItemList = sectionInfo.SectionItemsAlt;
						iSectionIndex = static_cast<int>(std::distance(sectionInfo.SectionList.begin(), sectionIterator));
						break;
					}
				}
				// not found
				if (iSectionIndex == -1) {
					logger::error("Could not find {} in the main or reserve arrays. Aborting sorter.", get_editorID(flSection));
					return false;
				}
				auto&            sSectionName = NameList[iSectionIndex];
				RE::BGSListForm* flItems = nullptr;
				if (ItemList.size() > iSectionIndex) {
					flItems = ItemList[iSectionIndex];
				}
				RE::BGSListForm* flItemsAlt = nullptr;
				if (AltItemList.size() > iSectionIndex) {
					flItemsAlt = AltItemList[iSectionIndex];
				}
				const auto actionInv = akActionRef->GetInventory();
				int        flItemsCount = 0, flItemsAltCount = 0;
				if (flItems != nullptr) {
					flItemsCount = getItemCount(actionInv, flItems);
				}
				if (flItemsAlt != nullptr) {
					flItemsAltCount = getItemCount(actionInv, flItemsAlt);
				}

				int iItemTotal = flItemsCount + flItemsAltCount;
				if (iItemTotal != 0) {
					logger::debug("Auto sorting {} in {} for a total item count of {}", sSectionName, sRoomName, iItemTotal);
					searchResult.emplace_back(
						ResultItem{
							flSection,
							flItems->As<RE::BGSListForm>(),
							flItemsAlt != nullptr ? flItemsAlt->As<RE::BGSListForm>() : nullptr,
							iItemTotal });
				}
			}
		}

		logger::debug("search result count: {}", searchResult.size());
		return true;
	}

	bool NextSectionSearchResult(STATIC_ARGS)
	{
		logger::debug("{} called", __FUNCTION__);
		if (resultAccessIndex < static_cast<int>(searchResult.size()) - 1) {
			++resultAccessIndex;
			return true;
		}
		return false;
	}

	std::vector<RE::BGSListForm*> GetSectionSearchFormList(STATIC_ARGS)
	{
		logger::debug("{} called", __FUNCTION__);
		std::vector<RE::BGSListForm*> ret;

		auto& item = searchResult.at(resultAccessIndex);
		ret.emplace_back(item.flSection);
		ret.emplace_back(item.flItems);
		ret.emplace_back(item.flItemsAlt);

		return ret;
	}

	int GetSectionSearchItemTotal(STATIC_ARGS)
	{
		logger::debug("{} called", __FUNCTION__);
		auto& item = searchResult.at(resultAccessIndex);

		return item.iItemTotal;
	}

	void ClearSectionSearch(STATIC_ARGS)
	{
		logger::debug("{} called", __FUNCTION__);
		SearchSections.clear();
		searchResult.clear();
		resultAccessIndex = -1;
	}

	void Bind(VM& a_vm)
	{
		BIND(CustomTransfer);
		BIND(AllTransfer);
		BIND(RelicTransfer);

		BIND(AddSearchSection);
		BIND(DBMSectionSearch);
		BIND(NextSectionSearchResult);
		BIND(GetSectionSearchFormList);
		BIND(GetSectionSearchItemTotal);
		BIND(ClearSectionSearch);
	}
}
