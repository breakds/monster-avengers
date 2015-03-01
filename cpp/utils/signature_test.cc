#include "utils/signature.h"
#include "aux/helpers.h"

using namespace monster_avengers;

int main() {
  // InverseKeyTest
  Signature key_a = sig::HolesToKey(4, 1, 0);
  key_a = sig::AddPoints(key_a, 0, 15);
  key_a = sig::AddPoints(key_a, 1, -2);
  key_a = sig::AddPoints(key_a, 2, 0);
  sig::ExplainSignature(key_a, {{46, 10}, {43, 10}, {91, 15}});

  Signature key_b = sig::HolesToKey(4, 0, 0);
  key_b = sig::AddPoints(key_b, 0, 2);
  key_b = sig::AddPoints(key_b, 1, 2);
  key_b = sig::AddPoints(key_b, 2, 0);
  sig::ExplainSignature(key_b, {{46, 10}, {43, 10}, {91, 15}});

  sig::ExplainSignature(sig::CombineKeyPoints(key_a, key_b),
                        {{46, 10}, {43, 10}, {91, 15}});

  std::vector<Effect> effects = {{46, 10}, {43, 10}, {91, 15}};

  Signature inverse_key = sig::InverseKey(effects.begin(), effects.end() - 1);
  sig::ExplainSignature(inverse_key,
                        {{46, 10}, {43, 10}, {91, 15}});

  CHECK(!sig::Satisfy(sig::CombineKeyPoints(key_a, key_b),
                      inverse_key));
  
  return 0;
}
