#include "DBMSortHandler.h"
#include "ClibUtil/editorID.hpp"
#include "Tools.h"

#define GET_FORMLIST_LOG(variable, editorID) GET_VARIANT(RE::BGSListForm, variable, editorID, "{} is invalid, {}:DBMSortHandler will not work")

namespace Papyrus::Functions::DBMSortHandler
{
	constexpr auto DBM_PROTECTEDITEMS = "DBM_ProtectedItems";
	constexpr auto DBM_EXCLUDELIST = "DBM_ExcludeList";
	constexpr auto DBM_REPLICAITEMS = "DBM_ReplicaItems";
	constexpr auto DBM_REPLICABASEITEMS = "DBM_ReplicaBaseItems";
	constexpr auto DBM_RADIANTRELICLIST = "DBM_RadiantRelicList";
	constexpr auto DBM_RADIANTBOOKLIST = "DBM_RadiantBookList";
	constexpr auto DBM_DISPLAYCOUNT = "DBM_DisplayCount";

	static RE::BGSListForm* DBM_ProtectedItems = nullptr;
	static RE::BGSListForm* DBM_ExcludeList = nullptr;
	static RE::BGSListForm* DBM_ReplicaItems = nullptr;
	static RE::BGSListForm* DBM_ReplicaBaseItems = nullptr;
	static RE::BGSListForm* DBM_RadiantRelicList = nullptr;
	static RE::BGSListForm* DBM_RadiantBookList = nullptr;
	static RE::TESGlobal*   DBM_DisplayCount = nullptr;

	bool init()
	{
		static bool           hasError = false;
		static std::once_flag callFlag;
		std::call_once(callFlag, []() {
			GET_FORMLIST_LOG(DBM_ProtectedItems, DBM_PROTECTEDITEMS);
			GET_FORMLIST_LOG(DBM_ExcludeList, DBM_EXCLUDELIST);
			GET_FORMLIST_LOG(DBM_ReplicaItems, DBM_REPLICAITEMS);
			GET_FORMLIST_LOG(DBM_ReplicaBaseItems, DBM_REPLICABASEITEMS);
			GET_FORMLIST_LOG(DBM_RadiantRelicList, DBM_RADIANTRELICLIST);
			GET_FORMLIST_LOG(DBM_RadiantBookList, DBM_RADIANTBOOKLIST);
			GET_VARIANT(RE::TESGlobal, DBM_DisplayCount, DBM_DISPLAYCOUNT, "{} is invalid, {}:DBMSortHandler will not work");
		});

		return !hasError;
	}

	void RemoveRadiantForm(RE::TESForm* form)
	{
		if (!init()) {
			return;
		}

		auto replicaIt = std::find(DBM_ReplicaItems->forms.begin(), DBM_ReplicaItems->forms.end(), form);
		if (replicaIt != DBM_ReplicaItems->forms.end()) {
			const int index = static_cast<int>(std::distance(DBM_ReplicaItems->forms.begin(), replicaIt));
			form = DBM_ReplicaBaseItems->forms[index];
			DBM_RadiantRelicList->RemoveAddedForm(form);
		} else {
			DBM_RadiantRelicList->RemoveAddedForm(form);
		}
		DBM_RadiantBookList->RemoveAddedForm(form);
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

	bool DBMSectionSearch(STATIC_ARGS, std::vector<RE::BGSListForm*> RoomList, std::vector<std::string> RoomNames, RE::TESObjectREFR* akActionRef)
	{
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

	static std::chrono::steady_clock::time_point BeginTimeStamp;

	void PerformanceCounterBegin(STATIC_ARGS)
	{
		logger::debug("{} called", __FUNCTION__);
		BeginTimeStamp = std::chrono::high_resolution_clock::now();
	}

	void PerformanceCounterEnd(STATIC_ARGS)
	{
		logger::debug("{} called", __FUNCTION__);
		auto end = std::chrono::high_resolution_clock::now();
		logger::info("{}ms cost", (end - BeginTimeStamp).count() / 1000LL / 1000LL);
	}

	int CheckDisplay(RE::TESObjectREFR* Disp, RE::TESForm* item, RE::TESForm* replica, RE::TESObjectREFR* oCont, RE::TESObjectREFR* akActionRef,
		bool bPreferReplicas, bool bOnlyReplicas, bool bQuestItemsProtected, bool useSKSE)
	{
		if (!init()) {
			return 0;
		}

		logger::debug("{} called", __FUNCTION__);
		using namespace clib_util::editorID;
		if (item == nullptr) {
			logger::info("Invalid item for display {}", Disp == nullptr ? "NULL" : Disp->GetName());
			return 0;
		} else if (Disp == nullptr) {
			logger::info("item {}:{} for Invalid display", item->GetName(), get_editorID(item));
			return 0;
		}

		if (replica != nullptr) {
			if (bOnlyReplicas || (bPreferReplicas && getItemCount(akActionRef->GetInventory(), replica) > 0)) {
				item = replica;
			}
		}

		bool bIgnoreItem = false;
		if (bQuestItemsProtected && DBM_ProtectedItems->HasForm(item) && getItemCount(akActionRef->GetInventory(), item) > 0) {
			logger::debug("{}:{} is a protected quest item and will not display automatically.", item->GetName(), get_editorID(item));
			bIgnoreItem = true;
		}

		if ((bOnlyReplicas || bPreferReplicas) && replica != nullptr) {
			auto displayReplica = [&]() {
				auto actor = akActionRef->As<RE::Actor>();
				if (Disp->IsDisabled() && getItemCount(akActionRef->GetInventory(), replica) > 0 && getItemCount(oCont->GetInventory(), replica) == 0 && (actor == nullptr || !IsEquipped(actor, replica)) && !DBM_ExcludeList->HasForm(replica) && (!useSKSE || !itemIsFavorited(replica))) {
					akActionRef->RemoveItem(static_cast<RE::TESBoundObject*>(replica), 1, RE::ITEM_REMOVE_REASON::kStoreInContainer, nullptr, oCont);
					DBM_DisplayCount->value += 1;
					Disp->Enable(false);
					logger::debug("Displayed replica {}:{} for {}", replica->GetName(), get_editorID(replica), Disp->GetName());
					if (useSKSE) {
						//!TODO DBMDebug.SendDisplayEvent(Self, Disp, Replica, true)
					}
					return 1;
				}
				return 0;
			};

			auto subList = replica->As<RE::BGSListForm>();
			if (subList != nullptr) {
				for (auto& subListItem : subList->forms) {
					replica = subListItem;
					if (displayReplica() == 1) {
						return 1;
					}
				}
			} else {
				if (displayReplica() == 1) {
					return 1;
				}
			}
		}

		if (!bIgnoreItem && !bOnlyReplicas && item != nullptr) {
			auto displayItem = [&]() {
				auto actor = akActionRef->As<RE::Actor>();
				if (Disp->IsDisabled() && getItemCount(akActionRef->GetInventory(), item) > 0 && getItemCount(oCont->GetInventory(), item) == 0 && (actor == nullptr || !IsEquipped(actor, item)) && !DBM_ExcludeList->HasForm(item) && (!useSKSE || !itemIsFavorited(item))) {
					akActionRef->RemoveItem(static_cast<RE::TESBoundObject*>(item), 1, RE::ITEM_REMOVE_REASON::kStoreInContainer, nullptr, oCont);
					RemoveRadiantForm(item);
					DBM_DisplayCount->value += 1;
					Disp->Enable(false);
					logger::debug("Displayed item {}:{} for {}", item->GetName(), get_editorID(item), Disp->GetName());
					if (useSKSE) {
						//!TODO DBMDebug.SendDisplayEvent(Self, Disp, item, true)
					}
					return 1;
				}
				return 0;
			};

			auto subList = item->As<RE::BGSListForm>();
			if (subList != nullptr) {
				for (auto& subListItem : subList->forms) {
					item = subListItem;
					if (displayItem() == 1) {
						return 1;
					}
				}
			} else {
				if (displayItem() == 1) {
					return 1;
				}
			}
		}

		return 0;
	}

	int SortDisplays_SKSE(STATIC_ARGS, RE::BGSListForm* flSection, RE::BGSListForm* flItems, int iItemTotal, RE::TESObjectREFR* oCont, RE::TESObjectREFR* akActionRef, bool bPreferReplicas, bool bOnlyReplicas, bool bQuestItemsProtected, bool useSKSE)
	{
		if (!init()) {
			return 0;
		}

		logger::debug("{} called", __FUNCTION__);
		int iNewDisplays = 0;
		for (int iCur = 0, count = flSection->forms.size(); iCur < count; ++iCur) {
			auto& fDisp = flSection->forms[iCur];
			auto  flDisp = fDisp->As<RE::BGSListForm>();

			if (flDisp != nullptr) {
				auto flItem = flItems->forms[iCur];

				for (int iDisps = flDisp->forms.size(); iDisps >= 0; --iDisps) {
					auto Disp = flDisp->forms[iDisps]->AsReference();
					auto item = flItem;
					auto flItemList = flItem->As<RE::BGSListForm>();
					if (flItemList != nullptr) {
						item = flItemList->forms[iDisps];
					}
					iItemTotal -= getItemCount(akActionRef->GetInventory(), item);
					auto replica = getReplica(item);
					if (replica != nullptr) {
						auto replicafl = replica->As<RE::BGSListForm>();
						if (replicafl != nullptr) {
							replica = replicafl->forms[iDisps];
						}
						iItemTotal -= getItemCount(akActionRef->GetInventory(), replica);
					}

					iNewDisplays += CheckDisplay(Disp, item, replica, oCont, akActionRef, bPreferReplicas, bOnlyReplicas, bQuestItemsProtected, useSKSE);
				}
			} else {
				auto Disp = fDisp->AsReference();
				auto item = flItems->forms[iCur];
				iItemTotal -= getItemCount(akActionRef->GetInventory(), item);
				auto replica = getReplica(item);
				if (replica != nullptr) {
					iItemTotal -= getItemCount(akActionRef->GetInventory(), replica);
				}
				iNewDisplays += CheckDisplay(Disp, item, replica, oCont, akActionRef, bPreferReplicas, bOnlyReplicas, bQuestItemsProtected, useSKSE);
			}

			if (iItemTotal <= 0) {
				break;
			}
		}

		return iNewDisplays;
	}

	void Bind(VM& a_vm)
	{
		BIND(AddSearchSection);
		BIND(DBMSectionSearch);
		BIND(NextSectionSearchResult);
		BIND(GetSectionSearchFormList);
		BIND(GetSectionSearchItemTotal);
		BIND(ClearSectionSearch);
		BIND(SortDisplays_SKSE);

		BIND(PerformanceCounterBegin);
		BIND(PerformanceCounterEnd);
	}
}
