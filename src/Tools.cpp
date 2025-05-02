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
