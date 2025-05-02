#include "DisplayHandler.h"
#include "ClibUtil/editorID.hpp"
#include "Tools.h"

#define GET_FORMLIST_LOG(variable, editorID) GET_VARIANT(RE::BGSListForm, variable, editorID, "{} is invalid, {}:DisplayHandler will not work")

namespace Papyrus::Functions::DisplayHandler
{
	constexpr auto ITEMSWITHVARIANTS = "TCCDisplayHelper_ItemsWithVariants";
	constexpr auto DBM_REPLICABASEITEMS = "DBM_ReplicaBaseItems";
	constexpr auto DBM_REPLICAITEMS = "DBM_ReplicaItems";
	constexpr auto DBMNEW = "dbmNew";
	constexpr auto DBMFOUND = "dbmFound";
	constexpr auto DBMDISP = "dbmDisp";
	constexpr auto DBMMASTER = "dbmMaster";
	constexpr auto _MUSEUMCONTAINERLIST = "_MuseumContainerList";

	static RE::BGSListForm*     ItemsWithVariants = nullptr;
	static RE::BGSListForm*     DBM_ReplicaBaseItems = nullptr;
	static RE::BGSListForm*     DBM_ReplicaItems = nullptr;
	static RE::BGSListForm*     dbmNew = nullptr;
	static RE::BGSListForm*     dbmFound = nullptr;
	static RE::BGSListForm*     dbmDisp = nullptr;
	static RE::BGSListForm*     dbmMaster = nullptr;
	static RE::BGSListForm*     _MuseumContainerList = nullptr;
	static std::recursive_mutex ReplicaMutex;

	static bool init()
	{
		using namespace RE;

		static bool           hasError = false;
		static std::once_flag callFlag;
		std::call_once(callFlag, []() {
			GET_FORMLIST_LOG(ItemsWithVariants, ITEMSWITHVARIANTS);
			GET_FORMLIST_LOG(DBM_ReplicaBaseItems, DBM_REPLICABASEITEMS);
			GET_FORMLIST_LOG(DBM_ReplicaItems, DBM_REPLICAITEMS);
			GET_FORMLIST_LOG(dbmNew, DBMNEW);
			GET_FORMLIST_LOG(dbmFound, DBMFOUND);
			GET_FORMLIST_LOG(dbmDisp, DBMDISP);
			GET_FORMLIST_LOG(dbmMaster, DBMMASTER);
			GET_FORMLIST_LOG(_MuseumContainerList, _MUSEUMCONTAINERLIST);

			const auto itemsWithVariantsSize = ItemsWithVariants->forms.size();
			const auto baseItemSize = DBM_ReplicaBaseItems->forms.size();
			const auto replicaSize = DBM_ReplicaItems->forms.size();
			if (baseItemSize != replicaSize || spdlog::get_level() <= spdlog::level::debug) {
				logger::info("================================={} Begin {}================================", ITEMSWITHVARIANTS, itemsWithVariantsSize);
				printFormList(ItemsWithVariants);
				logger::info("================================={} End================================\n", ITEMSWITHVARIANTS);

				logger::info("================================={} Begin {}================================", DBM_REPLICABASEITEMS, baseItemSize);
				printFormList(DBM_ReplicaBaseItems);
				logger::info("================================={} End================================\n", DBM_REPLICABASEITEMS);

				logger::info("================================={} Begin {}================================", DBM_REPLICAITEMS, replicaSize);
				printFormList(DBM_ReplicaItems);
				logger::info("================================={} End================================", DBM_REPLICAITEMS);

				if (baseItemSize != replicaSize) {
					RE::DebugMessageBox(std::format("{}: potential error: {} has {} records, {} has {} records",
						Version::PROJECT,
						DBM_REPLICABASEITEMS,
						baseItemSize,
						DBM_REPLICAITEMS,
						replicaSize)
							.c_str());
				}
			}
		});

		return !hasError;
	}

	void ReplicaAndVariantsAddHandler(STATIC_ARGS, RE::TESForm* a_form)
	{
		if (!init()) {
			return;
		}

		using namespace RE;
		using namespace clib_util::editorID;

		logger::debug("{} called: {} added; thread {}", __FUNCTION__, a_form->GetName(), std::format("{}", std::this_thread::get_id()));
		if (a_form == nullptr) {
			logger::error("{}: invalid item", __FUNCTION__);
			return;
		}

		// (DBM) Items with Replica
		bool isReplica = false;
		auto dbmIndex = FormListLevel2Search(a_form, DBM_ReplicaBaseItems);
		if (dbmIndex == -1) {  // not a replica base item
			dbmIndex = FormListLevel2Search(a_form, DBM_ReplicaItems);
			isReplica = true;
		}
		if (dbmIndex != -1) {
			logger::debug("{} is a {}", a_form->GetName(), isReplica ? "replica item" : "replica base item");

			std::lock_guard<std::recursive_mutex> locker(ReplicaMutex);

			{  // add origin variants
				auto* baseForm = DBM_ReplicaBaseItems->forms[dbmIndex];
				if (baseForm->GetFormType() == FormType::FormList) {
					logger::debug("adding replica base item list: {}", get_editorID(baseForm));
					auto* baseFormList = static_cast<RE::BGSListForm*>(baseForm);
					FormListAdd(dbmDisp, baseFormList);
					FormListSub(dbmFound, baseFormList);
					FormListSub(dbmNew, baseFormList);
				} else if (!dbmDisp->HasForm(baseForm)) {
					logger::debug("adding replica base item: {}", baseForm->GetName());
					dbmDisp->AddForm(baseForm);
					dbmFound->RemoveAddedForm(baseForm);
					dbmNew->RemoveAddedForm(baseForm);
				}
			}

			{  // add replica variants
				auto* replicaForm = DBM_ReplicaItems->forms[dbmIndex];
				if (replicaForm->GetFormType() == FormType::FormList) {
					logger::debug("adding replica item list: {}", get_editorID(replicaForm));
					auto* replicaFormList = static_cast<RE::BGSListForm*>(replicaForm);
					FormListAdd(dbmDisp, replicaFormList);
					FormListSub(dbmFound, replicaFormList);
					FormListSub(dbmNew, replicaFormList);
				} else if (!dbmDisp->HasForm(replicaForm)) {
					logger::debug("adding replica item: {}", replicaForm->GetName());
					dbmDisp->AddForm(replicaForm);
					dbmFound->RemoveAddedForm(replicaForm);
					dbmNew->RemoveAddedForm(replicaForm);
				}
			}
			return;
		}

		// Other items with variants
		const auto variantIndex = FormListLevel2Search(a_form, ItemsWithVariants);
		if (variantIndex != -1) {
			auto* variantList = ItemsWithVariants->forms[variantIndex];
			if (variantList->GetFormType() == FormType::FormList) {
				std::lock_guard<std::recursive_mutex> locker(ReplicaMutex);
				logger::debug("adding variants: {}", get_editorID(variantList));
				auto* formList = static_cast<RE::BGSListForm*>(variantList);
				FormListAdd(dbmDisp, formList);
				FormListSub(dbmFound, formList);
				FormListSub(dbmNew, formList);
			}
		}
	}

	void ReplicaAndVariantsRemoveHandler(STATIC_ARGS, RE::TESForm* a_form, std::vector<RE::TESObjectREFR*> TokenRefList, RE::TESObjectREFR* akContainer)
	{
		if (!init()) {
			return;
		}

		using namespace RE;
		using namespace clib_util::editorID;

		logger::debug("{} called: {} removed; thread {}", __FUNCTION__, a_form->GetName(), std::format("{}", std::this_thread::get_id()));
		if (a_form == nullptr) {
			logger::error("{}: invalid item", __FUNCTION__);
			return;
		}

		// (DBM) Items with Replica
		bool isReplica = false;
		auto dbmIndex = FormListLevel2Search(a_form, DBM_ReplicaBaseItems);
		if (dbmIndex == -1) {  // not a replica base item
			dbmIndex = FormListLevel2Search(a_form, DBM_ReplicaItems);
			isReplica = true;
		}
		if (dbmIndex != -1) {
			logger::debug("{} is a {}", a_form->GetName(), isReplica ? "replica item" : "replica base item");

			std::lock_guard<std::recursive_mutex> locker(ReplicaMutex);
			{  // remove all origin items
				auto* baseForm = DBM_ReplicaBaseItems->forms[dbmIndex];
				if (baseForm->GetFormType() == FormType::FormList) {
					logger::debug("removing replica base item list: {}", get_editorID(baseForm));
					auto* baseFormList = static_cast<RE::BGSListForm*>(baseForm);
					FormListSub(dbmDisp, baseFormList);

					if (std::find(TokenRefList.begin(), TokenRefList.end(), akContainer) == TokenRefList.end()) {
						FormListAdd(dbmNew, baseFormList);
					} else {
						FormListAdd(dbmFound, baseFormList);
					}
				} else if (dbmDisp->HasForm(baseForm)) {
					logger::debug("removing replica base item: {}", baseForm->GetName());
					dbmDisp->RemoveAddedForm(baseForm);

					if (std::find(TokenRefList.begin(), TokenRefList.end(), akContainer) == TokenRefList.end()) {
						dbmNew->AddForm(baseForm);
					} else {
						dbmFound->AddForm(baseForm);
					}
				}
			}

			{  // remove all replica items
				auto* replicaForm = DBM_ReplicaItems->forms[dbmIndex];
				if (replicaForm->GetFormType() == FormType::FormList) {
					logger::debug("removing replica item list: {}", get_editorID(replicaForm));
					auto* replicaFormList = static_cast<RE::BGSListForm*>(replicaForm);
					FormListSub(dbmDisp, replicaFormList);

					if (std::find(TokenRefList.begin(), TokenRefList.end(), akContainer) == TokenRefList.end()) {
						FormListAdd(dbmNew, replicaFormList);
					} else {
						FormListAdd(dbmFound, replicaFormList);
					}
				} else if (dbmDisp->HasForm(replicaForm)) {
					logger::debug("removing replica base item: {}", replicaForm->GetName());
					dbmDisp->RemoveAddedForm(replicaForm);

					if (std::find(TokenRefList.begin(), TokenRefList.end(), akContainer) == TokenRefList.end()) {
						dbmNew->AddForm(replicaForm);
					} else {
						dbmFound->AddForm(replicaForm);
					}
				}
			}
			return;
		}

		// Other items with variants
		const auto variantIndex = FormListLevel2Search(a_form, ItemsWithVariants);
		if (variantIndex != -1) {
			auto* variantList = ItemsWithVariants->forms[variantIndex];
			if (variantList->GetFormType() == FormType::FormList) {
				logger::debug("removing variants: {}", get_editorID(variantList));
				auto* formlist = static_cast<RE::BGSListForm*>(variantList);
				FormListSub(dbmDisp, formlist);

				if (std::find(TokenRefList.begin(), TokenRefList.end(), akContainer) == TokenRefList.end()) {
					FormListAdd(dbmNew, formlist);
				} else {
					FormListAdd(dbmFound, formlist);
				}
			}
		}
	}

	void RepopulateDbmNew(STATIC_ARGS)
	{
		if (!init()) {
			return;
		}
		logger::debug("{} called", __FUNCTION__);

		std::lock_guard<std::recursive_mutex> locker(ReplicaMutex);

		int       i = 0;
		const int total = dbmMaster->forms.size();
		for (; i < total; ++i) {
			dbmNew->AddForm(dbmMaster->forms[i]);
			if (i % 3000 == 0) {
				RE::DebugNotification(std::format("The Curators Companion: Rebuilding moreHUD lists... ({}/{})", i, total).c_str(), nullptr, false);
			}
		}
		RE::DebugNotification(std::format("The Curators Companion: Rebuilding moreHUD lists finished ({}/{})", i, total).c_str(), nullptr, false);
	}

	void updateMoreHUDLists(STATIC_ARGS, std::vector<RE::TESObjectREFR*> TokenRefList)
	{
		if (!init()) {
			return;
		}
		logger::debug("{} called", __FUNCTION__);

		using namespace clib_util::editorID;

		std::lock_guard<std::recursive_mutex> locker(ReplicaMutex);

		for (const auto& container : _MuseumContainerList->forms) {
			auto* containerRef = container->AsReference();
			if (containerRef == nullptr) {
				logger::warn("{} is invalid reference", container->GetFormEditorID());
				continue;
			}

			auto inv = containerRef->GetInventory();
			for (const auto& [item, data] : inv) {
				const auto& [numItem, entry] = data;
				auto* form = entry->GetObject()->As<RE::TESForm>();
				if (!dbmDisp->HasForm(form)) {
					if (dbmNew->HasForm(form) || dbmFound->HasForm(form)) {
						dbmNew->RemoveAddedForm(form);
						dbmFound->RemoveAddedForm(form);
						dbmDisp->AddForm(form);
						ReplicaAndVariantsAddHandler(PASS_STATIC_ARGS, form);
					}
				}
			}
		}

		for (auto& container : TokenRefList) {
			if (container == nullptr) {
				continue;
			}
			auto inv = container->GetInventory();
			for (const auto& [item, data] : inv) {
				const auto& [numItem, entry] = data;
				auto* form = entry->GetObject()->As<RE::TESForm>();
				if (dbmNew->HasForm(form) && !dbmDisp->HasForm(form)) {
					dbmNew->RemoveAddedForm(form);
					dbmFound->AddForm(form);
				}
			}
		}

		logger::debug("{}: dbmNew = {}", __FUNCTION__, dbmNew->forms.size());
		logger::debug("{}: dbmFound = {}", __FUNCTION__, dbmFound->forms.size());
		logger::debug("{}: dbmDisp = {}", __FUNCTION__, dbmDisp->forms.size());
		logger::debug("{}: dbmMaster = {}", __FUNCTION__, dbmMaster->forms.size());
	}

	void Bind(VM& a_vm)
	{
		BIND(FormListLevel2Search);
		BIND(FormListAdd);
		BIND(FormListSub);
		BIND(ReplicaAndVariantsAddHandler);
		BIND(ReplicaAndVariantsRemoveHandler);
		BIND(RepopulateDbmNew);
		BIND(updateMoreHUDLists);

		logger::info("TCCDisplayHelper Registered form functions"sv);
	}
}
