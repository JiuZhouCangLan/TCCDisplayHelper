#include "TransferHandler.h"
#include "Tools.h"

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
				if (numItem <= 0) {
					continue;
				}
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

	void Bind(VM& a_vm)
	{
		BIND(CustomTransfer);
		BIND(AllTransfer);
		BIND(RelicTransfer);
	}
}
