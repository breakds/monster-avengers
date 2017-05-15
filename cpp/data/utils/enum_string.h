#pragma once

#include <string>

namespace monster_avengers {

template <typename EnumType>
struct EnumString {
  static const std::wstring& Text(EnumType value) {
    static_assert("Unimplemented enum type for EnumString!");
    return L"";
  }
  const EnumType Parse(const std::string& text) {
    static_assert("Unimplemented enum type for EnumString!");
    return 0;
  }
};

}  // namespace monster_avengers
