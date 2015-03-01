#ifndef _MONSTER_AVENGERS_SKILL_
#define _MONSTER_AVENGERS_SKILL_

#include <vector>

#include "effect.h"
#include "language_text.h"

#include "lisp/lisp_object.h"

namespace monster_avengers {

  struct Skill {
    int points;
    LanguageText name;
    LanguageText description;

    Skill(const lisp::Object &object) {
      object.AssignSlotTo("NAME", &name);
      object.AssignSlotTo("POINTS", &points);
      object.AssignSlotTo("DESCRIPTION", &description);
    }

    void DebugPrint(int indent = 0) const {
      for (int i = 0; i < indent; ++i) wprintf(L" ");
      wprintf(L"Skill {\n");
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"name: %ls\n", name.jp.c_str());
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"description: %ls\n", description.en.c_str());
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"points: %d\n", points);
      for (int i = 0; i < indent; ++i) wprintf(L" ");
      wprintf(L"};\n");
    }
  };

  struct SkillSystem {
    LanguageText name;
    std::vector<Skill> skills;
    int id;
    
    explicit SkillSystem(const lisp::Object &object) {
      object.AssignSlotTo("NAME", &name);
      object.AssignSlotTo("ID", &id);
      object.AppendSlotTo("SKILLS", &skills);
    }
    
    void DebugPrint(int indent = 0) const {
      for (int i = 0; i < indent; ++i) wprintf(L" ");
      wprintf(L"SkillSystem {\n");
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"name: %ls\n", name.jp.c_str());
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"skills: [\n");
      for (const Skill &skill : skills) {
        skill.DebugPrint(indent + 4);
      }
      for (int i = 0; i < indent + 2; ++i) wprintf(L" ");
      wprintf(L"]\n");
      for (int i = 0; i < indent; ++i) wprintf(L" ");
      wprintf(L"};\n");
    }
  };
  
}  // namespace monster_avengers


#endif  // _MONSTER_AVENGERS_SKILL_
