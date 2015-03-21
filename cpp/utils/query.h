#ifndef _MONSTER_AVENGERS_QUERY_
#define _MONSTER_AVENGERS_QUERY_

#include <string>
#include <unordered_map>
#include <unordered_set>

#include "supp/helpers.h"
#include "lisp/parser.h"
#include "data/data_set.h"

namespace monster_avengers {

  struct Query {

    enum Command {
      SKILL = 0,
      DEFENSE,
      WEAPON_TYPE,
      WEAPON_HOLES,
      MIN_RARE,
      MAX_RARE,
      ADD_AMULET,
      MAX_RESULTS,
      BLACKLIST,
    };

    static const std::unordered_map<std::wstring, Command> COMMAND_TRANSLATOR;
    
    std::vector<Effect> effects;
    int defense;
    WeaponType weapon_type;
    int weapon_holes;
    int min_rare;
    int max_rare;
    int max_results;
    std::vector<Armor> amulets;
    std::unordered_set<int> blacklist;

    Query() : effects(), defense(0), weapon_type(MELEE) {}

    // Implies conversion from string as well.
    static Status Parse(const std::wstring &query_text, Query *query) {
      query->defense = 0;
      query->weapon_type = MELEE;
      query->effects.clear();
      query->weapon_holes = 0; // by default do not allow weapon holes.
      query->min_rare = 0; // by default there is no rare limit.
      query->max_rare = 11; // by default there is no rare limit.
      query->max_results = 10; // by default we are expecting 10 results.
      query->amulets.clear();

      auto tokenizer = lisp::Tokenizer::FromText(query_text);
      lisp::Token token;
      Command command;
      Status status(SUCCESS);
      int skill_id = 0;
      int skill_points = 0;
      std::vector<Effect> effects;
      std::vector<int> nums;
      int holes;

      while (tokenizer.Next(&token)) {
        // Get "("
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
          query->effects.emplace_back(skill_id, skill_points);
          break;
        case DEFENSE:
          status = ReadInt(&tokenizer, &query->defense);
          if (!status.Success()) return status;
          break;
        case WEAPON_TYPE:
          status = ReadWeaponType(&tokenizer, &query->weapon_type);
          if (!status.Success()) return status;
          break;
        case WEAPON_HOLES:
          status = ReadInt(&tokenizer, &query->weapon_holes);
          if (!status.Success()) return status;
          break;
	case MIN_RARE:
	  status = ReadInt(&tokenizer, &query->min_rare);
          if (!status.Success()) return status;
          break;
        case MAX_RARE:
	  status = ReadInt(&tokenizer, &query->max_rare);
          if (!status.Success()) return status;
          break;
        case MAX_RESULTS:
	  status = ReadInt(&tokenizer, &query->max_results);
          if (!status.Success()) return status;
          break;
        case ADD_AMULET:
          status = ReadInt(&tokenizer, &holes);
          if (!status.Success()) return status;
          nums.clear();
          effects.clear();
          nums = lisp::ParseList<int>::Do(&tokenizer);
          for (int i = 0; i < (nums.size() >> 1); ++i) {
            effects.emplace_back(nums[i << 1], nums[(i << 1) + 1]);
          }
          query->amulets.push_back(Armor::Amulet(holes, effects));
          break;
        case BLACKLIST:
          nums.clear();
          nums = lisp::ParseList<int>::Do(&tokenizer);
          for (int i : nums) {
            query->blacklist.insert(i);
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
        Log(ERROR, L"error while opening %s.", file_name.c_str());
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
      weapon_type = other.weapon_type;
      return *this;
    }

    bool HasSkill(int skill_id) const {
      for (const Effect &effect : effects) {
        if (skill_id == effect.skill_id) return true;
      }
      return false;
    }
    
    void DebugPrint() {
      wprintf(L"---------- Query ----------\n");
      wprintf(L"skill:");
      for (const Effect &effect :effects) {
        wprintf(L" %d(%d)", effect.skill_id, effect.points);
      }
      wprintf(L"\n");
      if (MELEE == weapon_type) {
        wprintf(L"weapon_type: MELEE\n");
      } else {
        wprintf(L"weapon_type: RANGE\n");
      }
      wprintf(L"weapon_holes: %d\n", weapon_holes);
      wprintf(L"mininum rare: %d\n", min_rare);
      wprintf(L"defense: %d\n", defense);
      for (auto &amulet : amulets) {
        amulet.DebugPrint();
      }
      wprintf(L"\n");
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
        *type = MELEE;
      } else if (L"range" == token.value) {
        *type = RANGE;
      } else {
        return Status(FAIL, "Query: Invalid weapon type.");
      }
      return Status(SUCCESS);
    }
  };

  const std::unordered_map<std::wstring, Query::Command> 
  Query::COMMAND_TRANSLATOR =
    {{L"skill", SKILL}, 
     {L"defense", DEFENSE}, 
     {L"weapon-type", WEAPON_TYPE},
     {L"weapon-holes", WEAPON_HOLES},
     {L"rare", MIN_RARE},
     {L"max-rare", MAX_RARE},
     {L"max-results", MAX_RESULTS},
     {L"amulet", ADD_AMULET},
     {L"blacklist", BLACKLIST}};
}

#endif  // _MONSTER_AVENGERS_QUERY_
