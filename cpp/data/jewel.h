#ifndef _MONSTER_AVENGERS_JEWEL_
#define _MONSTER_AVENGERS_JEWEL_

#include <string>

#include "effect.h"
#include "language_text.h"
#include "lisp/lisp_object.h"

namespace monster_avengers {
  struct Jewel {
    LanguageText name;
    int holes;
    std::vector<Effect> effects;
    int external_id;

    Jewel() : name(), holes(0), effects(), external_id(0) {}

    Jewel(const lisp::Object &object) {
      object.AssignSlotTo("NAME", &name);
      object.AssignSlotTo("SLOTS", &holes);
      object.AssignSlotTo("EXTERNAL-ID", &external_id, -1);
      object.AppendSlotTo("EFFECTS", &effects);
    }

    void DebugPrint(int indent = 0) const {
      for (int i = 0; i < indent; ++i) wprintf(L" ");
      wprintf(L"Jewel {\n");
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"name (JP): %ls\n", name.jp.c_str());
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"name (EN): %ls\n", name.en.c_str());
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"holes: %d\n", holes);
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"effects: [\n");
      for (const Effect &effect : effects) {
        effect.DebugPrint(indent + 4);
      }
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"]\n");
      for (int i = 0; i < indent; ++i) wprintf(L" ");
      wprintf(L"};\n");
    }
  };
}  // namespace monster_avengers


#endif  // _MONSTER_AVENGERS_JEWEL_
