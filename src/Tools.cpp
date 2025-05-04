#include "Tools.h"

void printFormList(RE::BGSListForm* formlist)
{
	using namespace clib_util::editorID;
	for (const auto& form : formlist->forms) {
		logger::info("{} {}", get_editorID(form), form->GetName());
	}
}

int FormListLevel2Search(RE::TESForm* a_form, RE::BGSListForm* a_list)
{
	logger::debug("{} called", __FUNCTION__);
	using namespace RE;

	if (a_form == nullptr || a_list == nullptr) {
		logger::error("FormListDeepSearch: invalid parameters");
		return false;
	}

	std::vector<std::pair<RE::BGSListForm*, int>> subFormLists;
	for (int i = 0, size = a_list->forms.size(); i < size; ++i) {
		auto* form = a_list->forms[i];
		if (form == a_form) {
			return i;
		} else if (form->GetFormType() == FormType::FormList) {
			subFormLists.push_back({ static_cast<RE::BGSListForm*>(form), i });
		}
	}

	for (const auto& [list, listIndex] : subFormLists) {
		const auto idIt = std::find(list->forms.begin(), list->forms.end(), a_form);
		if (idIt != list->forms.end()) {
			return listIndex;
		}
	}

	return -1;
}

void FormListAdd(RE::BGSListForm* target, RE::BGSListForm* source)
{
	logger::debug("{} called", __FUNCTION__);
	for (const auto& form : source->forms) {
		if (!target->HasForm(form)) {
			target->AddForm(form);
		}
	}
}

void FormListSub(RE::BGSListForm* target, RE::BGSListForm* source)
{
	logger::debug("{} called", __FUNCTION__);
	for (const auto& form : source->forms) {
		target->RemoveAddedForm(form);
	}
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

int getItemCount(const RE::TESObjectREFR::InventoryItemMap& inventory, RE::TESForm* form)
{
	for (const auto& [item, data] : inventory) {
		const auto& [count, entry] = data;
		if (item == form) {
			return count;
		}
	}
	return 0;
}

int getItemCount(const RE::TESObjectREFR::InventoryItemMap& inventory, RE::BGSListForm* formlist)
{
	int total = 0;
	for (const auto& [item, data] : inventory) {
		const auto& [count, entry] = data;
		if (formlist->HasForm(item)) {
			total += count;
		}
	}
	return total;
}

constexpr auto          DBM_REPLICABASEITEMS = "DBM_ReplicaBaseItems";
constexpr auto          DBM_REPLICAITEMS = "DBM_ReplicaItems";
static RE::BGSListForm* DBM_ReplicaBaseItems = nullptr;
static RE::BGSListForm* DBM_ReplicaItems = nullptr;

// c++ implementation for Function GetReplica in LOTD dbm_replicahandler.psc
RE::TESForm* GetReplica(RE::TESForm* form)
{
	static bool           hasError = false;
	static std::once_flag callFlag;
	std::call_once(callFlag, []() {
		GET_VARIANT(RE::BGSListForm, DBM_ReplicaBaseItems, DBM_REPLICABASEITEMS, "{} is invalid, {}:getReplica will not work");
		GET_VARIANT(RE::BGSListForm, DBM_ReplicaItems, DBM_REPLICAITEMS, "{} is invalid, {}:getReplica will not work");
	});

	if (hasError) {
		return nullptr;
	}

	auto&      baseItems = DBM_ReplicaBaseItems->forms;
	const auto baseIt = std::find(baseItems.begin(), baseItems.end(), form);
	if (baseIt == baseItems.end()) {
		return nullptr;
	}
	const int index = static_cast<int>(std::distance(baseItems.begin(), baseIt));

	return DBM_ReplicaItems->forms[index];
}

bool itemIsFavorited(RE::TESForm* form)
{
	const auto player = RE::PlayerCharacter::GetSingleton();

	if (player == nullptr) {
		logger::error("can not get player instance");
		return false;
	}

	auto inv = player->GetInventory();
	for (const auto& [item, data] : inv) {
		const auto& [count, entry] = data;
		if (form == item) {
			return entry->IsFavorited();
		}
	}

	return false;
}

// call papyrus Function SendDisplayEvent in LOTD dbmdebug.psc
void SendDisplayEvent(Papyrus::VM* vm, RE::TESForm* fSender, RE::TESObjectREFR* oDisplay, RE::TESForm* fItem, bool bEnabled)
{
	RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> result;

	auto args = RE::MakeFunctionArguments(
		std::move(fSender),
		std::move(oDisplay),
		std::move(fItem),
		std::move(bEnabled));
	const bool callRet = vm->DispatchStaticCall("DBMDebug", "SendDisplayEvent", args, result);
	if (!callRet) {
		logger::error("call SendDisplayEvent fail");
	}
}
