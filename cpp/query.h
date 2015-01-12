#ifndef _MONSTER_AVENGERS_QUERY_
#define _MONSTER_AVENGERS_QUERY_

#include "helpers.h"
#include "parser.h"
#include "monster_hunter_data.h"

namespace monster_avengers {

  struct Query {
    enum Command {
      SKILL = 0,
      DEFENSE,
      WEAPON_TYPE
    };
    
    std::vector<Effect> effects;
    int defense;
    WeaponType weapon_type;

    Query() : effects(), defense(0), weapon_type(MELEE) {}

    // Implies conversion from string as well.
    static Status Parse(const std::wstring &query_text, Query *query) {
      query->defense = 0;
      query->weapon_type = MELEE;
      query->effects.clear();
      auto tokenizer = parser::Tokenizer::FromText(query_text);
      parser::Token token;
      Command command;
      Status status(SUCCESS);
      int skill_id = 0;
      int skill_points = 0;
      while (tokenizer.Next(&token)) {
        // Get "("
        if (parser::OPEN_PARENTHESIS != token.name) {
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
        default:
          return Status(FAIL, "Query: Invalid command.");
        }
        ExpectCloseParen(&tokenizer);
      }
      return Status(SUCCESS);
    }

    const Query &operator=(const Query &other) {
      effects = other.effects;
      defense = other.defense;
      weapon_type = other.weapon_type;
      return *this;
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
      wprintf(L"defense: %d\n\n", defense);
    }

  private:
    static Status ReadInt(parser::Tokenizer *tokenizer, int *number) {
      parser::Token token;
      if (!tokenizer->Next(&token)) {
        return Status(FAIL, "Query: Unexpected end of query.");
      }
      if (parser::NUMBER != token.name) {
        return Status(FAIL, "Query: Syntax Error - expect NUMBER.");
      }
      *number = std::stoi(token.value);
      return Status(SUCCESS);
    }
    
    static Status ExpectCloseParen(parser::Tokenizer *tokenizer) {
      parser::Token token;
      if (!tokenizer->Next(&token)) {
        return Status(FAIL, "Query: Unexpected end of query.");
      }
      if (parser::CLOSE_PARENTHESIS != token.name) {
        return Status(FAIL, "Query: Syntax Error - "
                      "expect CLOSE_PARENTHESIS.");
      }
      return Status(SUCCESS);
    }

    static Status ReadCommand(parser::Tokenizer *tokenizer, Command *command) {
      parser::Token token;
      if (!tokenizer->Next(&token)) {
        return Status(FAIL, "Query: Unexpected end of query.");
      }
      if (parser::KEYWORD != token.name) {
        return Status(FAIL, "Query: Syntax Error - expect KEYWORD.");
      }
      if (L"skill" == token.value) {
        *command = SKILL;
      } else if (L"defense" == token.value) {
        *command = DEFENSE;
      } else if (L"weapon-type" == token.value) {
        *command = WEAPON_TYPE;
      } else {
        return Status(FAIL, "Query: Invalid command.");
      }
      return Status(SUCCESS);
    }

    static Status ReadWeaponType(parser::Tokenizer *tokenizer, 
                                 WeaponType *type) {
      parser::Token token;
      if (!tokenizer->Next(&token)) {
        return Status(FAIL, "Query: Unexpected end of query.");
      }
      if (parser::STRING != token.name) {
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
}

#endif  // _MONSTER_AVENGERS_QUERY_
