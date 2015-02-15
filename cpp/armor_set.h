#ifndef _MONSTER_AVENGERS_ARMOR_SET_
#define _MONSTER_AVENGERS_ARMOR_SET_

#include <cstdio>
#include <cwchar>
#include <string>
#include <fstream>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <functional>

namespace monster_avengers {

  const int MAX_JEWEL_PLAN = 5;

  struct ArmorSet {
    std::array<int, PART_NUM> ids;
    std::vector<Signature> jewel_keys;
  };
  
  void OutputArmorSet(const DataSet &data, 
                      const ArmorSet &armor_set, 
                      const JewelSolver &solver) {
    wprintf(L"---------- Armor Set ----------\n");
    int defense = 0;
    std::vector<Effect> effects;
    const std::array<int, PART_NUM> &ids = armor_set.ids;
    for (int i = 0; i < PART_NUM; ++i) {
      const Armor &armor = data.armor(ids[i]);
      defense += data.armor(ids[i]).defense;
      wprintf(L"[");
      for (int j = 0; j < 3; ++j) {
        if (j < armor.holes) {
          wprintf(L"O");
        } else {
          wprintf(L"-");
        }
      }
      if (GEAR == armor.part) {
        wprintf(L"] [Rare ??] ??? %ls (%d)\n", 
                armor.name.c_str(), ids[i]);
      } else {
        if (armor.multiplied) {
          const Armor &base = data.armor(armor.base);
          wprintf(L"] [Rare %02d] %s %ls (%d) ",
                  base.rare,
                  (MELEE == base.type) ? "--H" : ")->", 
                  base.name.c_str(), armor.base);
          for (auto item : armor.jewels) {
            wprintf(L"%d x %ls ", item.second,
                    data.jewel(item.first).name.c_str());
          }
          wprintf(L"\n");
        } else {
          wprintf(L"] [Rare %02d] %s %ls (%d) %ls\n", 
                  armor.rare,
                  (MELEE == armor.type) ? "--H" : ")->", 
                  armor.name.c_str(), ids[i],
                  data.ProvidesTorsoUp(ids[i]) ?
                  data.skill_system(data.torso_up_id).name.c_str() : L"");
        }
      }
      for (const Effect &effect : armor.effects) {
        auto it = std::find_if(effects.begin(), effects.end(),
                               [&effect](const Effect& x) {
                                 return x.skill_id == effect.skill_id;
                               });
        if (effects.end() == it) {
          effects.push_back(effect);
        } else {
          it->points += effect.points;
        }
      }
    }
    wprintf(L"Defense: %d\n", defense);
    for (const Effect &effect : effects) {
      wprintf(L"%ls(%d)  ", 
              data.skill_system(effect.skill_id).name.c_str(),
              effect.points);
    }
    wprintf(L"\n");

    int plan_count = 0;
    for (const Signature &jewel_key : armor_set.jewel_keys) {
      wprintf(L"Jewel Plan:    ");
      for (auto item : solver.Solve(jewel_key)) {
        wprintf(L"%d x %ls  ", item.second, 
                data.jewel(item.first).name.c_str());
      }
      wprintf(L"\n");
      if (++plan_count >= MAX_JEWEL_PLAN) {
        break;
      }
    }
    wprintf(L"\n");
  }

  void PrettyPrintArmorSet(const DataSet &data, 
			   const ArmorSet &armor_set, 
			   const JewelSolver &solver) {
    int defense = 0;
    std::vector<Effect> effects;
    const std::array<int, PART_NUM> &ids = armor_set.ids;
    for (int i = 0; i < PART_NUM; ++i) {
      const Armor &armor = data.armor(ids[i]);
      defense += data.armor(ids[i]).defense;
      wprintf(L"[");
      for (int j = 0; j < 3; ++j) {
        if (j < armor.holes) {
          wprintf(L"O");
        } else {
          wprintf(L"-");
        }
      }
      if (GEAR == armor.part) {
        wprintf(L"] [Rare ??] ??? %ls\n", 
                armor.name.c_str());
      } else if (AMULET == armor.part) {
        wprintf(L"] [Rare ??] ??? %ls    ",
                armor.name.c_str());
	for (const Effect &effect : armor.effects) {
	  wprintf(L"%ls(%d) ", 
		  data.skill_system(effect.skill_id).name.c_str(),
		  effect.points);
	}
	wprintf(L"\n");
      } else {
        wprintf(L"] [Rare %02d] %s %ls (id: %d)     Require:",
                armor.rare,
                (MELEE == armor.type) ? "--H" : ")->", 
		armor.name.c_str(), ids[i]);

        for (const std::wstring &material_name : armor.material) {
          wprintf(L"  %ls", material_name.c_str());
        }
        wprintf(L"\n");
      }
      for (const Effect &effect : armor.effects) {
        auto it = std::find_if(effects.begin(), effects.end(),
                               [&effect](const Effect& x) {
                                 return x.skill_id == effect.skill_id;
                               });
        if (effects.end() == it) {
          effects.push_back(effect);
        } else {
          it->points += effect.points;
        }
      }
    }
    wprintf(L"Defense: %d\n", defense);
    for (const Effect &effect : effects) {
      wprintf(L"%ls(%d)  ", 
              data.skill_system(effect.skill_id).name.c_str(),
              effect.points);
    }
    wprintf(L"\n");
    int plan_count = 0;
    for (const Signature &jewel_key : armor_set.jewel_keys) {
      wprintf(L"Jewel Plan:    ");
      for (auto item : solver.Solve(jewel_key)) {
        wprintf(L"%d x %ls  ", item.second, 
                data.jewel(item.first).name.c_str());
      }
      wprintf(L"\n");
      if (++plan_count >= MAX_JEWEL_PLAN) {
        break;
      }
    }
    wprintf(L"\n");
  }

  
  struct LispObject {
    typedef std::unique_ptr<void, std::function<void(void*)>> DataHolder;
    typedef std::unordered_map<std::string, LispObject> ObjectHolder;

    enum LispObjectType {
      LISP_NUM = 0,
      LISP_STR = 1,
      LISP_OBJ = 2,
      LISP_LIST = 3,
    };
    LispObjectType type;
    DataHolder data;

  LispObject() : data(nullptr), type(LISP_OBJ) {}

    LispObject(LispObject &&other) {
      type = other.type;
      data = std::move(other.data);
    }

    LispObject(int x) :
      type(LISP_NUM), data(new int(x), [](void * content) {
	  delete static_cast<int*>(content);}) {}

    LispObject(const std::wstring &x) :
      type(LISP_STR), data(new std::wstring(x), [](void * content) {
	  delete static_cast<std::wstring*>(content);}) {}
    
    const LispObject &operator=(LispObject &&other) {
      type = other.type;
      data = std::move(other.data);
      return (*this);
    }
    
    static LispObject Number(int x = 0) {
      return LispObject(x);
    }

    static LispObject String(const std::wstring &x) {
      return LispObject(x);
    }

    static LispObject Object() {
      LispObject result;
      result.type = LISP_OBJ;
      result.data = DataHolder(new ObjectHolder(), [](void * content) {
	  delete static_cast<ObjectHolder*>(content);});
      return result;
    }

    static LispObject List() {
      LispObject result;
      result.type = LISP_LIST;
      result.data = DataHolder(new std::vector<LispObject>(), [](void * content) {
	  delete static_cast<std::vector<LispObject>*>(content);});
      return result;
    }

    void Set(const std::string name, LispObject &&value) {
      CHECK(LISP_OBJ == this->type);
      (*static_cast<ObjectHolder*>(data.get()))[name] = std::move(value);
    }

    void Push(LispObject &&value) {
      CHECK(LISP_LIST == this->type);
      static_cast<std::vector<LispObject>*>(data.get())->emplace_back(std::move(value));
    }

    LispObject &operator[](const std::string &name) {
      CHECK(LISP_OBJ == this->type);
      return (*static_cast<ObjectHolder*>(data.get()))[name];
    }

    LispObject &operator[](int id) {
      CHECK(LISP_LIST == this->type);
      return (*static_cast<std::vector<LispObject>*>(data.get()))[id];
    }

    int Size() const {
      return static_cast<std::vector<LispObject>*>(data.get())->size();
    }

    ObjectHolder &GetMap() {
      return (*static_cast<ObjectHolder*>(data.get()));
    }
  };

  std::wostream &operator<<(std::wostream &out, LispObject &lisp_object) {
    
    switch (lisp_object.type) {
    case LispObject::LISP_NUM: 
      out << *static_cast<int*>(lisp_object.data.get()); 
      break;
    case LispObject::LISP_STR: 
      out << "\"" << *static_cast<std::wstring*>(lisp_object.data.get())
	  << "\"";
      break;
    case LispObject::LISP_LIST:
      out << "(";
      for (int i = 0; i < lisp_object.Size(); ++i) {
	if (i > 0) out << " ";
	out << lisp_object[i];
      }
      out << ")";
      break;
    case LispObject::LISP_OBJ:
      out << "(";
      int i = 0;
      for (auto &item : lisp_object.GetMap()) {
	std::wstring name;
	name.assign(item.first.begin(), item.first.end());
	if (i > 0) out << " ";
	out << ":" << name
	    << " " << item.second;
	i++;
      }
      out << ")";
      break;
    }
    return out;
  }

  class ArmorSetFormatter {
  public:
    ArmorSetFormatter(const std::string file_name, 
                      const DataSet *data,
                      const Query &query)
      : output_stream_(new std::wofstream(file_name)),
        solver_(*data, query.effects), 
        data_(data) {
      if (!output_stream_->good()) {
        Log(ERROR, L"error while opening %s.", file_name.c_str());
        exit(-1);
      }
      output_stream_->imbue(std::locale("en_US.UTF-8"));
    }

    void Format(const ArmorSet &armor_set) {
      LispObject output = LispObject::Object();
      int defense = 0;
      std::unordered_map<int, int> effects;
      for (int i = 0; i < PART_NUM; ++i) {
        ArmorPart part = static_cast<ArmorPart>(PART_NUM - i - 1);
        int id = armor_set.ids[i];
        const Armor &armor = data_->armor(id);
	output.Set(PartName(part),
		   GetArmorObject(armor, part, id));
        defense += armor.defense;
        for (const Effect &effect : armor.effects) {
          auto it = effects.find(effect.skill_id);
          if (effects.end() == it) {
            effects[effect.skill_id] = effect.points;
          } else {
            it->second += effect.points;
          }
        }
      }

      output.Set("defense", defense);

      output.Set("jewel-plans", LispObject::List());

      int jewel_plan_count = 0;
      for (const Signature &jewel_key : armor_set.jewel_keys) {
        if (jewel_plan_count < MAX_JEWEL_PLAN) {
	  LispObject jewel_plan_object = LispObject::Object();
	  jewel_plan_object.Set("plan", LispObject::List());
          std::unordered_map<int, int> jewel_plan_effects = effects;
          for (auto item : solver_.Solve(jewel_key)) {
	    LispObject plan_object = LispObject::Object();
            const Jewel &jewel = data_->jewel(item.first);
	    plan_object.Set("name", jewel.name);
	    plan_object.Set("quantity", item.second);
	    jewel_plan_object["plan"].Push(std::move(plan_object));
            for (const Effect &effect : jewel.effects) {
              auto it = jewel_plan_effects.find(effect.skill_id);
              if (jewel_plan_effects.end() == it) {
                jewel_plan_effects[effect.skill_id] = 
                  effect.points * item.second;
              } else {
                it->second += effect.points * item.second;
              }
            }
          }
	  jewel_plan_object.Set("active", GetActiveObject(jewel_plan_effects));
          output["jewel-plans"].Push(std::move(jewel_plan_object));
        }
        jewel_plan_count++;
      }
      
      (*output_stream_) << output << "\n";
    }
    
  private:
    std::string PartName(ArmorPart part) {
      switch (part) {
      case HEAD: return "head"; 
      case BODY: return "body";
      case HANDS: return "hands";
      case WAIST: return "waist";
      case FEET: return "feet";
      case AMULET: return "amulet";
      case GEAR: return "gear";
      default: Log(ERROR, L"FormatArmor: no such armor part %d", part); exit(-1); break;
      }
    }
    
    LispObject GetArmorObject(const Armor &armor, ArmorPart part, int id) {
      LispObject armor_object = LispObject::Object();
      armor_object["name"] = armor.name;
      armor_object["holes"] = armor.holes;
      armor_object["id"] = std::to_wstring(id);
      if (AMULET == part) {
	armor_object.Set("effects", GetEffectsObject(armor.effects));
      } else if (GEAR != part) {
	armor_object.Set("material", GetMaterialObject(armor.material));
	armor_object.Set("rare", armor.rare);
      }        
      return armor_object;
    }

    LispObject GetEffectsObject(const std::vector<Effect> &effects) {
      LispObject result = LispObject::List();
      for (const Effect &effect : effects) {
	LispObject effect_object = LispObject::Object();
	effect_object.Set("name", data_->skill_system(effect.skill_id).name);
	effect_object.Set("points", effect.points); 
	result.Push(std::move(effect_object));
      }
      return result;
    }

    LispObject GetMaterialObject(const std::vector<std::wstring> &material) {
      LispObject result = LispObject::List();
      for (const std::wstring &item : material) {
	result.Push(item);
      }
      return result;
    }

    LispObject GetActiveObject(const std::unordered_map<int, int> &effects) {
      LispObject active_object = LispObject::List();
      for (auto &effect : effects) {
        const SkillSystem &skill_system = data_->skill_system(effect.first);
        int active_points = 0;
        std::wstring active_name = L"";
        if (effect.second > 0) {
          for (const Skill &skill : skill_system.skills) {
            if (effect.second > 0) {
              if (effect.second >= skill.points && 
                  skill.points > active_points) {
                active_points = skill.points;
                active_name = skill.name;
              }
            } else if (effect.second <= skill.points && 
                       skill.points < active_points) {
              active_points = skill.points;
              active_name = skill.name;
            }
          }
        }
        if (!active_name.empty()) {
	  active_object.Push(active_name);
        }
      }
      return active_object;
    }

    std::unique_ptr<std::wofstream> output_stream_;
    const JewelSolver solver_;
    const DataSet *data_;
  };
}  // namespace monster_avengers


#endif  // _MONSTER_AVENGERS_ARMOR_SET_

