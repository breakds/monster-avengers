#ifndef _MONSTER_AVENGERS_LANGUAGE_TEXT_
#define _MONSTER_AVENGERS_LANGUAGE_TEXT_

#include <string>
#include "lisp/lisp_object.h"

namespace monster_avengers {
  struct LanguageText {
    std::wstring en;
    std::wstring jp;

    LanguageText() : en(L""), jp(L"") {}

    LanguageText(const lisp::Object &object) {
      object.AssignSlotTo("EN", &en);
      object.AssignSlotTo("JP", &jp);
    }

    LanguageText(LanguageText &&other) {
      en.swap(other.en);
      jp.swap(other.jp);
    }

    const LanguageText &operator=(LanguageText &&other) {
      en.swap(other.en);
      jp.swap(other.jp);
      return *this;
    }

    const wchar_t *c_str() const {
      return jp.c_str();
    }
  };
}  // namespace monster_avengers


#endif  // _MONSTER_AVENGERS_LANGUAGE_TEXT_
