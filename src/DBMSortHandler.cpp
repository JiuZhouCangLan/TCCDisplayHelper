#include "DBMSortHandler.h"
#include "Tools.h"

namespace Papyrus::Functions::DBMSortHandler
{
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

	void Bind(VM& a_vm)
	{
		BIND(AddSearchSection);
		BIND(DBMSectionSearch);
		BIND(NextSectionSearchResult);
		BIND(GetSectionSearchFormList);
		BIND(GetSectionSearchItemTotal);
		BIND(ClearSectionSearch);

		BIND(PerformanceCounterBegin);
		BIND(PerformanceCounterEnd);
	}
}
