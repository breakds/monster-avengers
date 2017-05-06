#include "base/strcat.h"
#include <string>
#include "gtest/gtest.h"

namespace base {
TEST(StrCatTest, WithStringLiterals) {
  // Test StrCat when all the arguments are string literals.
  EXPECT_EQ("This is a concatenated string.",
            StrCat("This", " ", "is", " ", "a", " ", "concatenated", " ",
                   "string", "."));
}
TEST(StrCatTest, WithCharLiterals) {
  // Test StrCat when the arguments are a mixture of both string and
  // char literals.
  EXPECT_EQ("This is a concatenated string.",
            StrCat("This", ' ', "is", ' ', "a", ' ', "concatenated", ' ',
                   "string", "."));
}
TEST(StrCatTest, WithAllVariables) {
  // Test str cat when the arguments are variables of different types.
  int a = 1;
  int b = 2;
  std::string plus_sign = " + ";
  EXPECT_EQ("1 + 2 = 3", StrCat(a, plus_sign, b, " = ", a + b));
}
}  // namespace base
