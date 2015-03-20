#ifndef _MONSTER_AVENGERS_OUTPUT_SPECS_
#define _MONSTER_AVENGERS_OUTPUT_SPECS_

#include <string>
#include <vector>
#include <unordered_map>
#include "lisp/lisp_object.h"
#include "data/language_text.h"
#include "data/armor.h"
#include "data/data_set.h"
#include "data/effect.h"
#include "jewels_query.h"
#include "signature.h"


namespace monster_avengers {
  
  constexpr int MAX_JEWEL_PLANS = 5;

  enum OutputSpec {
    LISP = 0,
    JSON = 1,
    SCREEN = 2,
  };
  
  struct ArmorSet {
    std::array<int, PART_NUM> ids;
    std::vector<Signature> jewel_keys;
  };

  struct AmuletEffect : public lisp::Formattable {
    LanguageText name;
    int points;

    AmuletEffect(const LanguageText &name_, int points_)
      : name(name_), points(points_) {}

    lisp::Object Format() const override {
      lisp::Object output = lisp::Object::Struct();
      output["name"] = name;
      output["points"] = points;
      return output;
    }
  };

  struct PackedArmor : public lisp::Formattable {
    LanguageText name;
    int holes;
    int id;
    int max_defense;
    int min_defense;
    int rare;
    Resistence resistence;
    std::wstring torso_up;
    std::vector<LanguageText> material;
    std::vector<AmuletEffect> effects;
    
    PackedArmor(const DataSet &data, int armor_id) {
      const Armor &armor = data.armor(armor_id);
      if (AMULET == armor.part) {
        name.en = L"----------";
        name.jp = L"----------";
        for (const Effect &effect : armor.effects) {
          effects.emplace_back(data.skill_system(effect.skill_id).name,
                               effect.points);
        }
      } else {
        name = armor.name;
      }
      holes = armor.holes;
      id = armor_id;
      max_defense = armor.max_defense;
      min_defense = armor.min_defense;
      rare = armor.rare;
      resistence = armor.resistence;
      torso_up = armor.TorsoUp() ? L"true" : L"false";
      for (int item : armor.material) {
        material.push_back(data.ItemName(item));
      }
    }
      
    lisp::Object Format() const override {
      lisp::Object output = lisp::Object::Struct();
      output["name"] = name;
      output["holes"] = holes;
      output["id"] = std::to_wstring(id);
      output["max-defense"] = max_defense;
      output["min-defense"] = min_defense;
      output["rare"] = rare;
      output["resistence"] = resistence;
      output["torsoup"] = torso_up;
      output["material"] = lisp::FormatList(material);
      output["effects"] = lisp::FormatList(effects);
      return output;
    }
  };


  struct JewelPair : public lisp::Formattable {
    LanguageText name;
    int quantity;

    JewelPair(const LanguageText &name_, int quantity_) 
      : name(name_), quantity(quantity_) {}

    lisp::Object Format() const override {
      lisp::Object output = lisp::Object::Struct();
      output["name"] = name;
      output["quantity"] = quantity;
      return output;
    }
  };

  namespace {
    void AcquireEffect(const Effect &effect, 
                       int multiplier,
                       std::unordered_map<int, int> *result) {
      auto it = result->find(effect.skill_id);
      if (result->end() == it) {
        (*result)[effect.skill_id] = effect.points * multiplier;
      } else {
        it->second += effect.points * multiplier;
      }
    }
    
    void AcquireEffects(const std::vector<Effect> &effects,
                        int multiplier,
                        std::unordered_map<int, int> *result) {
      for (const Effect &effect : effects) {
        AcquireEffect(effect, multiplier, result);
      }
    }
  };

  struct SummaryItem : public lisp::Formattable {
    LanguageText name;
    int points;
    LanguageText active;

    SummaryItem(const LanguageText &name_,
                int points_,
                const LanguageText &active_) 
      : name(name_), points(points_), active(active_) {}
    
    lisp::Object Format() const override {
      lisp::Object output = lisp::Object::Struct();
      output["name"] = name;
      output["points"] = points;
      output["active"] = active;
      return output;
    }
  };

  struct JewelPlan : public lisp::Formattable {
    std::vector<JewelPair> plan;
    std::vector<JewelPair> body_plan;
    std::vector<SummaryItem> summary;
    int body_stuffed;

    JewelPlan(const DataSet &data,
              const JewelSolver::JewelPlan &jewel_plan,
              const int multiplier,
              const std::unordered_map<int, int> &base_effects) {
      std::unordered_map<int ,int> effects = base_effects;

      // Ordinary Jewels
      for (auto item : jewel_plan.first) {
        const Jewel &jewel = data.jewel(item.first);
        AcquireEffects(jewel.effects, item.second, &effects);
        plan.emplace_back(jewel.name, item.second);
      }

      // Body Jewels
      body_stuffed = 0;
      for (auto item : jewel_plan.second) {
        const Jewel &jewel = data.jewel(item.first);
        AcquireEffects(jewel.effects, multiplier * item.second, &effects);
        body_plan.emplace_back(jewel.name, item.second);
        body_stuffed += jewel.holes * item.second;
      }

      // Summary
      for (auto &effect : effects) {
        const SkillSystem &system = data.skill_system(effect.first);
        int active_id = system.FindActive(effect.second);
        if (active_id > -1) {
          LanguageText active = system.skills[active_id].name;
          summary.emplace_back(system.name, effect.second, active);
        } else {
          summary.emplace_back(system.name, effect.second, LanguageText());
        }
      }      
    }
    
    lisp::Object Format() const override {
      lisp::Object output = lisp::Object::Struct();
      output["plan"] = lisp::FormatList(plan);
      output["body-plan"] = lisp::FormatList(body_plan);
      output["body-stuffed"] = body_stuffed;
      output["summary"] = lisp::FormatList(summary);
      return output;
    }
  };

  struct ArmorResult : public lisp::Formattable {
    PackedArmor head;
    PackedArmor body;
    PackedArmor hands;
    PackedArmor waist;
    PackedArmor feet;
    PackedArmor amulet;
    PackedArmor gear;
    int defense;
    std::vector<JewelPlan> plans;

    ArmorResult(const DataSet &data, 
                const JewelSolver &solver, 
                const ArmorSet &armor_set) 
      : head(data, armor_set.ids[PART_NUM - HEAD - 1]),
        body(data, armor_set.ids[PART_NUM - BODY - 1]),
        hands(data, armor_set.ids[PART_NUM - HANDS - 1]),
        waist(data, armor_set.ids[PART_NUM - WAIST - 1]),
        feet(data, armor_set.ids[PART_NUM - FEET - 1]),
        amulet(data, armor_set.ids[PART_NUM - AMULET - 1]),
        gear(data, armor_set.ids[PART_NUM - GEAR - 1]),
        defense(std::accumulate(armor_set.ids.begin(),
                                armor_set.ids.end(),
                                0,
                                [&data](int accu, int id) {
                                  return accu + data.armor(id).max_defense;
                                })) {
      int multiplier = 
        std::accumulate(armor_set.ids.begin(),
                        armor_set.ids.end(),
                        1, // initially we have 1 for body itself
                        [&data](int accu, int id) {
                          return accu + (data.armor(id).TorsoUp() ? 1 : 0);
                        });
      std::unordered_map<int, int> effects;
      
      // Base Effects
      for (int i = 0; i < PART_NUM; ++i) {
        const Armor &armor = data.armor(armor_set.ids[i]);
        AcquireEffects(armor.effects, 
                       armor.part == BODY ? multiplier : 1,
                       &effects);        
      }

      // Combine Jewel Effects
      for (const Signature &jewel_key : armor_set.jewel_keys) {
        if (plans.size() >= MAX_JEWEL_PLANS) break;
        plans.emplace_back(data, solver.Solve(jewel_key, multiplier), 
                           multiplier,
                           effects);
      }
    }

    lisp::Object Format() const override {
      lisp::Object output = lisp::Object::Struct();
      output["head"] = head;
      output["body"] = body;
      output["hands"] = hands;
      output["waist"] = waist;
      output["feet"] = feet;
      output["amulet"] = amulet;
      output["gear"] = gear;
      output["defense"] = defense;
      output["jewel-plans"] = lisp::FormatList(plans);
      return output;
    }
  };

  // ---------- JSON ----------

  // Amulet
  struct JsonTalisman : public lisp::Formattable {
    int slot;
    std::vector<Effect> effects;
    
    JsonTalisman(const DataSet &data, int armor_id) {
      const Armor &amulet = data.armor(armor_id);
      slot = amulet.holes;
      effects = amulet.effects;
    }

    lisp::Object Format() const override {
      lisp::Object output = lisp::Object::Struct();
      output["slot"] = slot;
      if (0 < effects.size()) {
        output["skilltree_1_id"] = effects[0].skill_id;
        output["skilltree_1_points"] = effects[0].points;
      }
      if (1 < effects.size()) {
        output["skilltree_2_id"] = effects[1].skill_id;
        output["skilltree_2_points"] = effects[1].points;
      }
      return output;
    }
  };

  struct JsonDecoration : public lisp::Formattable {
    int jewel_id;
    int quantity;
    bool on_body;

    JsonDecoration(int jewel_id_, int quantity_, bool on_body_) 
      : jewel_id(jewel_id_), quantity(quantity_), on_body(on_body_) {}

    
    lisp::Object Format() const override {
      lisp::Object output = lisp::Object::Struct();
      output["jewel_id"] = jewel_id;
      output["quantity"] = quantity;
      output["on_body"] = std::wstring(on_body ? L"true" : L"false");
      return output;
    }
  };

  struct JsonArmorResult : public lisp::Formattable {
    int head_id;
    int body_id;
    int arms_id;
    int waist_id;
    int legs_id;
    int weapon_slot;
    JsonTalisman talisman;
    std::vector<std::vector<JsonDecoration> > plans;
    
    JsonArmorResult(const DataSet &data, 
                    const JewelSolver &solver, 
                    const ArmorSet &armor_set) 
      : talisman(data, armor_set.ids[AMULET]) {
      weapon_slot = data.armor(armor_set.ids[GEAR]).holes;
      head_id = armor_set.ids[HEAD];
      body_id = armor_set.ids[BODY];
      arms_id = armor_set.ids[HANDS];
      waist_id = armor_set.ids[WAIST];
      legs_id = armor_set.ids[FEET];

      int multiplier = 
        std::accumulate(armor_set.ids.begin(),
                        armor_set.ids.end(),
                        1, // initially we have 1 for body itself
                        [&data](int accu, int id) {
                          return accu + (data.armor(id).TorsoUp() ? 1 : 0);
                        });
      
      plans.clear();
      for (const Signature &jewel_key : armor_set.jewel_keys) {
        if (plans.size() >= MAX_JEWEL_PLANS) break;
        plans.emplace_back();
        const JewelSolver::JewelPlan jewel_plan = 
          std::move(solver.Solve(jewel_key, multiplier));
        for (const auto &item : jewel_plan.first) {
          plans.back().emplace_back(item.first, item.second, false);
        }

        // On-body jewels
        for (const auto &item : jewel_plan.second) {
          plans.back().push_back({item.first, item.second, true});
        }
      }
    }

    lisp::Object Format() const override {
      lisp::Object output = lisp::Object::Struct();
      output["head_id"] = head_id;
      output["body_id"] = body_id;
      output["arms_id"] = arms_id;
      output["waist_id"] = waist_id;
      output["legs_id"] = legs_id;
      output["weapon_slot"] = weapon_slot;
      output["talisman"] = talisman;
      output["decoration_plans"] = lisp::FormatList(plans);
      return output;
    }
  };

  // ---------- Explore ----------

  struct ExploreResult : public lisp::Formattable {
    std::vector<int> ids;
    
    ExploreResult() : ids() {}

    lisp::Object Format() const override {
      return lisp::FormatList(ids);
    }
  };

}  // namespace monster_avengers



#endif  // _MONSTER_AVENGERS_OUTPUT_SPECS_
