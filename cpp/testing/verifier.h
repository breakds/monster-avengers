// This is the library for verifying the search results.

#ifndef _MONSTER_AVENGERS_CORE_VERIFIER_
#define _MONSTER_AVENGERS_CORE_VERIFIER_

#include "dataset/dataset.h"

using namespace monster_avengers::dataset;

using monster_avengers::dataset::StringifyEnum;

namespace monster_avengers {

namespace testing {

struct VerificationMessage {
  std::string tag;
  std::wstring detail;

  template <typename... AnyType>
  VerificationMessage(const std::string &input_tag,
                      const wchar_t *format,
                      AnyType... any) {
    Set(input_tag, format, any...);
  }

  void Set(const std::string &input_tag, const wchar_t *format, ...) {
    static const int buffer_size = 255;
    static wchar_t buffer[buffer_size];

    tag = input_tag;

    va_list argptr;
    va_start(argptr, format);
    vswprintf(buffer, buffer_size, format, argptr);
    va_end(argptr);

    detail = buffer;
  }

  void Print() const {
    Log(FATAL, L"On [%s]:  %ls\n", tag.c_str(), detail.c_str());
  }
};

bool VerifySkillRequirement(const Arsenal &arsenal,
                            const Query &query,
                            const ArmorSet &armor_set,
                            std::vector<VerificationMessage> *messages) {
  std::vector<Effect> stats = std::move(Data::GetSkillStats(
      armor_set, arsenal));

  bool pass = true;
  
  for (const Effect &effect : query.effects) {
    bool found = false;
    for (const Effect &stat : stats) {
      if (stat.id == effect.id) {
        found = true;
        if (stat.points >= effect.points) {
          break;
        }

        // Fail Case 1: Cannot found the skill.
        messages->emplace_back(
            "SKILL", L"%ls(%d) is not fufilled (%d/%d).",
            Data::GetSkillName(effect.id).c_str(),
            effect.id, stat.points, effect.points);
        pass = false;
      }
    }
    if (!found) {
      messages->emplace_back(
          "SKILL", L"%ls(%d) is expected but not found in result.",
          Data::GetSkillName(effect.id).c_str(), effect.id);
      pass = false;
    }
  }

  // Check negative skill
  if (query.avoid_negative) {
    for (const Effect &stat : stats) {
      if (Data::NegativeActivated(stat)) {
        messages->emplace_back(
            "SKILL", L"%ls(%d) is negatively activated! (%d).",
            Data::GetSkillName(stat.id).c_str(),
            stat.id, stat.points);
        pass = false;
      }
    }
  }
  
  return pass;
}

bool VerifyDefenseRequirement(const Arsenal &arsenal,
                              const Query &query,
                              const ArmorSet &armor_set,
                              std::vector<VerificationMessage> *messages) {
  int defense = Data::GetTotalDefense(armor_set, arsenal);
  if (defense < query.defense) {
    messages->emplace_back(
        "DEFENSE", L"Defense = %d which does not meet the target (%d).",
        defense, query.defense);
    return false;
  }
  return true;
}

bool VerifyWeaponTypeRequirement(const Arsenal &arsenal,
                                 const Query &query,
                                 const ArmorSet &armor_set,
                                 std::vector<VerificationMessage> *messages) {
  bool pass = true;
  
  for (int i = 0; i < PART_NUM; ++i) {
    const Armor &armor = arsenal[armor_set.ids[i]];
    if (armor.weapon_type == (1 - query.armor_filter.weapon_type)) {
      messages->emplace_back(
          "WEAPON TYPE", L"Piece %ls(%d) is for %s, but %s is required.",
          Data::GetArmorName(armor_set.ids[i]).c_str(), armor_set.ids[i],
          StringifyEnum(armor.weapon_type).c_str(),
          StringifyEnum(query.armor_filter.weapon_type).c_str());
      pass = false;
    }
  }

  return pass;
}

bool VerifyGenderRequirement(const Arsenal &arsenal,
                             const Query &query,
                             const ArmorSet &armor_set,
                             std::vector<VerificationMessage> *messages) {
  bool pass = true;
  
  for (int i = 0; i < PART_NUM; ++i) {
    const Armor &armor = arsenal[armor_set.ids[i]];
    if (armor.gender == (1 - query.armor_filter.gender)) {
      messages->emplace_back(
          "GENDER", L"Piece %ls(%d) is for %s, but %s is required.",
          Data::GetArmorName(armor_set.ids[i]).c_str(), armor_set.ids[i],
          StringifyEnum(armor.gender).c_str(),
          StringifyEnum(query.armor_filter.gender).c_str());
      pass = false;
    }
  }

  return pass;
}

bool VerifyRareRequirement(const Arsenal &arsenal,
                           const Query &query,
                           const ArmorSet &armor_set,
                           std::vector<VerificationMessage> *messages) {
  bool pass = true;
  
  for (int i = 0; i < PART_NUM; ++i) {
    if (GEAR == i || AMULET == i) continue;

    const Armor &armor = arsenal[armor_set.ids[i]];
    if (armor.rare < query.armor_filter.min_rare ||
        armor.rare > query.armor_filter.max_rare) {
      messages->emplace_back(
          "RARE", L"Piece %ls(%d) has rare %d which does not fall in [%d, %d].",
          Data::GetArmorName(armor_set.ids[i]).c_str(), armor_set.ids[i],
          armor.rare, query.armor_filter.min_rare, query.armor_filter.max_rare);
      pass = false;
    }
  }

  return pass;
}


// Verify that armor blacklists and whitelists are respected.
bool VerifyArmorBlacklist(const Arsenal &arsenal,
                          const Query &query,
                          const ArmorSet &armor_set,
                          std::vector<VerificationMessage> *messages) {
  bool pass = true;

  for (int i = 0; i < PART_NUM; ++i) {
    if (GEAR == i || AMULET == i) continue;

    int id = armor_set.ids[i];
    if (0 != query.armor_filter.blacklist.count(id) ||
        (0 < query.armor_filter.whitelist.size() &&
         0 == query.armor_filter.whitelist.count(id))) {
      messages->emplace_back(
          "BLACKLIST/WHITELIST",
          L"Piece %ls(%d) is backlisted, or not whitelisted.",
          Data::GetArmorName(id).c_str(), id);
      pass = false;
    }
  }
  return pass;
}

bool VerifyJewelBlacklist(const Arsenal &arsenal,
                          const Query &query,
                          const ArmorSet &armor_set,
                          std::vector<VerificationMessage> *messages) {
  bool pass = true;

  for (int i = 0; i < PART_NUM; ++i) {
    for (const int jewel_id : armor_set.jewels[i]) {
      if (0 != query.jewel_filter.blacklist.count(jewel_id)) {
        messages->emplace_back(
          "JEWEL BLACKLIST",
          L"Jewel %ls(%d) is present, but should be blacklisted.",
          Data::GetJewelName(jewel_id).c_str(), jewel_id);
        pass = false;
      }
    }
  }
  return pass;
}

std::vector<VerificationMessage> VerifyArmorSet(const Arsenal &arsenal,
                                                const Query &query,
                                                const ArmorSet &armor_set) {
  std::vector<VerificationMessage> messages;
  
  VerifyWeaponTypeRequirement(arsenal, query, armor_set, &messages);
  VerifyGenderRequirement(arsenal, query, armor_set, &messages);
  VerifyArmorBlacklist(arsenal, query, armor_set, &messages);
  VerifyJewelBlacklist(arsenal, query, armor_set, &messages);
  VerifyRareRequirement(arsenal, query, armor_set, &messages);
  VerifyDefenseRequirement(arsenal, query, armor_set, &messages);
  VerifySkillRequirement(arsenal, query, armor_set, &messages);
  
  return messages;
}

}  // namespace testing

}  // namespace monster_avengers


#endif  //  _MONSTER_AVENGERS_CORE_VERIFIER_
