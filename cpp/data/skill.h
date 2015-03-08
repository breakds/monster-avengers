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

    int FindActive(int points) const {
      int active_points = 0;
      int active_id = -1;
      for (int i = 0; i < skills.size(); ++i) {
        if ((points > 0 && points >= skills[i].points && 
             skills[i].points > active_points) ||
            (points < 0 && points <= skills[i].points &&
             skills[i].points < active_points)) {
          active_points = skills[i].points;
          active_id = i;
        }
      }
      return active_id;
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
