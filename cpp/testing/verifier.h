// This is the library for verifying the search results.

#ifndef _MONSTER_AVENGERS_CORE_VERIFIER_
#define _MONSTER_AVENGERS_CORE_VERIFIER_

#include "dataset/dataset.h"

using namespace monster_avengers::dataset;

namespace monster_avengers {

namespace testing {

struct VerificationMessage {
  std::string tag;
  std::string detail;
};

bool VerifySkillRequirement(const Arsenal &arsenal,
                            const Query &query,
                            const ArmorSet &armor_set,
                            VerificationMessage *message) {
  std::vector<Effect> stats = std::move(Data::GetSkillStats(
      armor_set, arsenal));
  for (const Effect &effect : query.effects) {
    bool found = false;
    for (const Effect &stat : stats) {
      if (stat.id == effect.id) {
        found = true;
        if (stat.points >= effect.points) {
          break;
        }

        // Fail Case 1: Cannot found the skill.
        message->tag = "skill";
        message->detail = "(" + std::to_string(effect.id) + ")" +
            " not fulfilled [" + std::to_string(stat.points) +
            "/" + std::to_string(effect.points) + "].";
        return false;
      }
    }
    if (!found) {
      message->tag = "skill";
      message->detail = "(" + std::to_string(effect.id) + ")" +
          " is expected but not found in result.";
      return false;
    }
  }
  return true;
}


std::vector<VerificationMessage> VerifyArmorSet(const Arsenal &arsenal,
                                                const Query &query,
                                                const ArmorSet &armor_set) {
  std::vector<VerificationMessage> result;
  VerificationMessage message;
  if (!VerifySkillRequirement(arsenal, query, armor_set, &message)) {
    result.push_back(message);
  }
  return result;
}

}  // namespace testing

}  // namespace monster_avengers


#endif  //  _MONSTER_AVENGERS_CORE_VERIFIER_
