#pragma once

#include "Common.h"

void printFormList(RE::BGSListForm* formlist);

int FormListLevel2Search(RE::TESForm* a_form, RE::BGSListForm* a_list);

void FormListAdd(RE::BGSListForm* target, RE::BGSListForm* source);

void FormListSub(RE::BGSListForm* target, RE::BGSListForm* source);

bool IsEquipped(RE::Actor* actor, RE::TESForm* object);

int getItemCount(const RE::TESObjectREFR::InventoryItemMap& inventory, RE::TESForm* form);

int getItemCount(const RE::TESObjectREFR::InventoryItemMap& inventory, RE::BGSListForm* formlist);

RE::TESForm* GetReplica(RE::TESForm* form);

bool itemIsFavorited(RE::TESForm* form);

void SendDisplayEvent(Papyrus::VM* vm, RE::TESForm* fSender, RE::TESObjectREFR* oDisplay, RE::TESForm* fItem, bool bEnabled);
