#ifndef _MONSTER_AVENGERS_EFFECT_
#define _MONSTER_AVENGERS_EFFECT_

#include "lisp/lisp_object.h"

namespace monster_avengers {
  struct Effect {
    int skill_id;
    int points;

    Effect(const lisp::Object &object) {
      object.AssignSlotTo("SKILL-ID", &skill_id);
      object.AssignSlotTo("POINTS", &points);
    }
    
    Effect(int input_skill_id, int input_points) 
      : skill_id(input_skill_id), points(input_points) {}

    void DebugPrint(int indent = 0) const {
      for (int i = 0; i < indent; ++i) wprintf(L" ");
      wprintf(L"Effect {\n");
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"skill_id: %d\n", skill_id);
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"points: %d\n", points);
      for (int i = 0; i < indent; ++i) wprintf(L" ");
      wprintf(L"};\n");
    }
  };
}  // namespace monster_avengers

#endif  // _MONSTER_AVENGERS_EFFECT_
