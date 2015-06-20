#ifndef _MONSTER_AVENGERS_UTILS_QUERY_
#define _MONSTER_AVENGERS_UTILS_QUERY_

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "supp/helpers.h"
#include "lisp/parser.h"
#include "dataset/dataset.h"

#include "filter.h"

using namespace monster_avengers::dataset;

namespace monster_avengers {

struct Query {

  enum Command {
    SKILL = 0,
    DEFENSE,
    WEAPON_TYPE,
    WEAPON_SLOTS,
    MIN_RARE,
    MAX_RARE,
    ADD_AMULET,
    MAX_RESULTS,
    BLACKLIST,
    JEWEL_BLACKLIST,
    GENDER,
    SPECIFY_ARMOR
  };

  static const std::unordered_map<std::wstring, Command> COMMAND_TRANSLATOR;
    
  std::vector<Effect> effects;
  std::vector<Armor> amulets;
  int defense;
  int max_results;
  ArmorFilter armor_filter;
  JewelFilter jewel_filter;
    
  Query() : effects(), defense(0), armor_filter() {}

  // Implies conversion from string as well.
  static Status Parse(const std::wstring &query_text, Query *query) {
    query->defense = 0;
    query->effects.clear();
    query->amulets.clear();
    query->max_results = 10; // by default we are expecting 10 results.

    // Armor Filter
    query->armor_filter.weapon_type = WEAPON_TYPE_MELEE;
    query->armor_filter.weapon_slots = 0; // by default do not allow weapon slots.
    query->armor_filter.min_rare = 0; // by default there is no rare limit.
    query->armor_filter.max_rare = 11; // by default there is no rare limit.
    query->armor_filter.gender = GENDER_MALE; // by default we look for male's armors.
    query->armor_filter.blacklist.clear();
    query->armor_filter.whitelist.clear();
      
    // Jewel Filter
    query->jewel_filter.blacklist.clear();


    auto tokenizer = lisp::Tokenizer::FromText(query_text);
    lisp::Token token;
    Command command;
    Status status(SUCCESS);
    int skill_id = 0;
    int skill_points = 0;
    std::vector<Effect> effects;
    std::vector<int> nums;
    int slots;
    Armor amulet;
    
    while (tokenizer.Next(&token)) {
      if (lisp::OPEN_PARENTHESIS != token.name) {
        return Status(FAIL, "Query: Syntax Error - expect '('.");
      }

      status = ReadCommand(&tokenizer, &command);
      if (!status.Success()) return status;

      switch (command) {
        case SKILL:
          status = ReadInt(&tokenizer, &skill_id);
          if (!status.Success()) return status;
          status = ReadInt(&tokenizer, &skill_points);
          if (!status.Success()) return status;
          query->effects.emplace_back();
          query->effects.back().id = Data::skills().Internalize(skill_id);
          query->effects.back().points = skill_points;
          break;
        case DEFENSE:
          status = ReadInt(&tokenizer, &query->defense);
          if (!status.Success()) return status;
          break;
        case WEAPON_TYPE:
          status = ReadWeaponType(&tokenizer, &query->armor_filter.weapon_type);
          if (!status.Success()) return status;
          break;
        case GENDER:
          status = ReadGender(&tokenizer, &query->armor_filter.gender);
          if (!status.Success()) return status;
          break;
        case WEAPON_SLOTS:
          status = ReadInt(&tokenizer, &query->armor_filter.weapon_slots);
          if (!status.Success()) return status;
          break;
	case MIN_RARE:
	  status = ReadInt(&tokenizer, &query->armor_filter.min_rare);
          if (!status.Success()) return status;
          break;
        case MAX_RARE:
	  status = ReadInt(&tokenizer, &query->armor_filter.max_rare);
          if (!status.Success()) return status;
          break;
        case MAX_RESULTS:
	  status = ReadInt(&tokenizer, &query->max_results);
          if (!status.Success()) return status;
          break;
        case ADD_AMULET:
          status = ReadInt(&tokenizer, &slots);
          if (!status.Success()) return status;
          nums.clear();
          amulet.effects.clear();
          nums = lisp::ParseList<int>::Do(&tokenizer);
          for (int i = 0; i < (nums.size() >> 1); ++i) {
            amulet.effects.emplace_back();
            amulet.effects.back().id = Data::skills().Internalize(nums[i << 1]);
            amulet.effects.back().points = nums[(i << 1) + 1];
          }

          // Create an amulet
          amulet.part = AMULET;
          amulet.weapon_type = WEAPON_TYPE_BOTH;
          amulet.gender = GENDER_BOTH;
          amulet.rare = 10;
          amulet.min_defense = 0;
          amulet.max_defense = 0;
          amulet.resistance = Resistance {0, 0, 0, 0, 0};
          amulet.slots = slots;
          query->amulets.push_back(amulet);
          break;
        case BLACKLIST:
          nums.clear();
          nums = lisp::ParseList<int>::Do(&tokenizer);
          for (int i : nums) {
            query->armor_filter.blacklist.insert(
                Data::armors().Internalize(i));
          }
          break;
        case SPECIFY_ARMOR:
          nums.clear();
          nums = lisp::ParseList<int>::Do(&tokenizer);
          for (int i : nums) {
            query->armor_filter.whitelist.insert(
                Data::armors().Internalize(i));
          }
          break;
        case JEWEL_BLACKLIST:
          nums.clear();
          nums = lisp::ParseList<int>::Do(&tokenizer);
          for (int i : nums) {
            query->jewel_filter.blacklist.insert(
                Data::jewels().Internalize(i));
          }
          break;
        default:
          return Status(FAIL, "Query: Invalid command.");
      }
      ExpectCloseParen(&tokenizer);
    }
    return Status(SUCCESS);
  }

  static Status ParseFile(const std::string file_name, Query *query) {
    std::wifstream input_stream(file_name);
    if (!input_stream.good()) {
      Log(FATAL, L"error while opening %s.", file_name.c_str());
      return Status(FAIL, "error while opening query file.");
    }
    std::wstring text = L"";
    wchar_t buffer;
    while (input_stream.get(buffer)) {
      text += buffer;
    }
    return Parse(text, query);
  }

  const Query &operator=(const Query &other) {
    effects = other.effects;
    defense = other.defense;
    max_results = other.max_results;
    amulets = other.amulets;
    armor_filter = other.armor_filter;
    return *this;
  }

  bool HasSkill(int skill_id) const {
    for (const Effect &effect : effects) {
      if (skill_id == effect.id) return true;
    }
    return false;
  }
    
 private:
  static Status ReadInt(lisp::Tokenizer *tokenizer, int *number) {
    lisp::Token token;
    if (!tokenizer->Next(&token)) {
      return Status(FAIL, "Query: Unexpected end of query.");
    }
    if (lisp::NUMBER != token.name) {
      return Status(FAIL, "Query: Syntax Error - expect NUMBER.");
    }
    *number = std::stoi(token.value);
    return Status(SUCCESS);
  }
    
  static Status ExpectCloseParen(lisp::Tokenizer *tokenizer) {
    lisp::Token token;
    if (!tokenizer->Next(&token)) {
      return Status(FAIL, "Query: Unexpected end of query.");
    }
    if (lisp::CLOSE_PARENTHESIS != token.name) {
      return Status(FAIL, "Query: Syntax Error - "
                    "expect CLOSE_PARENTHESIS.");
    }
    return Status(SUCCESS);
  }

  static Status ReadCommand(lisp::Tokenizer *tokenizer, Command *command) {
    lisp::Token token;
    if (!tokenizer->Next(&token)) {
      return Status(FAIL, "Query: Unexpected end of query.");
    }
      
    if (lisp::KEYWORD != token.name) {
      return Status(FAIL, "Query: Syntax Error - expect KEYWORD.");
    }

    auto it = COMMAND_TRANSLATOR.find(token.value);

    if (COMMAND_TRANSLATOR.end() != it) {
      *command = it->second;
    } else {
      return Status(FAIL, "Query: Invalid command.");
    }
    return Status(SUCCESS);
  }

  static Status ReadWeaponType(lisp::Tokenizer *tokenizer, 
                               WeaponType *type) {
    lisp::Token token;
    if (!tokenizer->Next(&token)) {
      return Status(FAIL, "Query: Unexpected end of query.");
    }
    if (lisp::STRING != token.name) {
      return Status(FAIL, "Query: Syntax Error - expect STRING.");
    }
    if (L"melee" == token.value) {
      *type = WEAPON_TYPE_MELEE;
    } else if (L"range" == token.value) {
      *type = WEAPON_TYPE_RANGE;
    } else {
      return Status(FAIL, "Query: Invalid weapon type.");
    }
    return Status(SUCCESS);
  }

  static Status ReadGender(lisp::Tokenizer *tokenizer, 
                           Gender *gender) {
    lisp::Token token;
    if (!tokenizer->Next(&token)) {
      return Status(FAIL, "Query: Unexpected end of query.");
    }
    if (lisp::STRING != token.name) {
      return Status(FAIL, "Query: Syntax Error - expect STRING.");
    }
    if (L"male" == token.value) {
      *gender = GENDER_MALE;
    } else if (L"female" == token.value) {
      *gender = GENDER_FEMALE;
    } else {
      return Status(FAIL, "Query: Invalid Gender.");
    }
    return Status(SUCCESS);
  }
    
};


// TODO(breakds): Use new Enum features to translate.
const std::unordered_map<std::wstring, Query::Command>
Query::COMMAND_TRANSLATOR =
{{L"skill", SKILL}, 
 {L"defense", DEFENSE}, 
 {L"weapon-type", WEAPON_TYPE},
 {L"weapon-slots", WEAPON_SLOTS},
 {L"rare", MIN_RARE},
 {L"max-rare", MAX_RARE},
 {L"max-results", MAX_RESULTS},
 {L"amulet", ADD_AMULET},
 {L"blacklist", BLACKLIST},
 {L"gender", GENDER},
 {L"specify-armor", SPECIFY_ARMOR},
 {L"ban-jewels", JEWEL_BLACKLIST},
};
}

#endif  // _MONSTER_AVENGERS_UTILS_QUERY_
