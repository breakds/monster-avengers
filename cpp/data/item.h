#ifndef _MONSTER_AVENGERS_ITEM_
#define _MONSTER_AVENGERS_ITEM_

#include <string>

#include "language_text.h"
#include "lisp/lisp_object.h"

namespace monster_avengers {
  struct Item {
    LanguageText name;
    int id;

    Item(const lisp::Object &object) {
      object.AssignSlotTo("NAME", &name);
      object.AssignSlotTo("ID", &id);
    }
    
    void DebugPrint(int indent = 0) const {
      for (int i = 0; i < indent; ++i) wprintf(L" ");
      wprintf(L"Item {\n");
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"%d: %ls  %ls\n", id,
              name.jp.c_str(), name.en.c_str());
      for (int i = 0; i < indent; ++i) wprintf(L" ");
      wprintf(L"}\n");
    }
  };
}  // namespace monster_avengers


#endif  // _MONSTER_AVENGERS_ITEM_
