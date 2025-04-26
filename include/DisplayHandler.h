#pragma once

#include "Common.h"

namespace Papyrus::Functions::DisplayHandler
{
	void printFormList(RE::BGSListForm* formlist);

	void FormListRemoveAddedForm(RE::BGSListForm* formlist, RE::TESForm* a_form);

	int FormListLevel2Search(STATIC_ARGS, RE::TESForm* a_form, RE::BGSListForm* a_list);

	void FormListAdd(STATIC_ARGS, RE::BGSListForm* target, RE::BGSListForm* source);

	void FormListSub(STATIC_ARGS, RE::BGSListForm* target, RE::BGSListForm* source);

	void Bind(VM& a_vm);
}
