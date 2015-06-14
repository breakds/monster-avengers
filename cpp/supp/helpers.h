#ifndef _MONSTER_AVENGERS_SUPP_HELPERS_H_
#define _MONSTER_AVENGERS_SUPP_HELPERS_H_

#include <cstdio>
#include <cwchar>
#include <locale>
#include <cstdarg>
#if _WIN32
#include <codecvt>
#endif

namespace monster_avengers {

// ---------- Locale ----------

extern const std::locale LOCALE_UTF8;
    
// ---------- Log ----------

enum LogLevel {
  INFO = 0,
  OK = 1,
  WARNING = 2,
  FATAL = 3,
};

void Log(LogLevel level, const wchar_t *format, ...);

// ---------- Status ----------

enum StatusName {
  SUCCESS = 0,
  FAIL = 1
};
  
class Status {
 public:
  Status (StatusName name, std::string error_message)
      : name_(name), error_message_(error_message) {}

  explicit Status(StatusName name) 
      : Status(name, "") {}

  const Status &operator=(const Status &other) {
    name_ = other.name_;
    error_message_ = other.error_message_;
    return *this;
  }

  inline bool Success() {
    return SUCCESS == name_;
  }

  const std::string &message() {
    return error_message_;
  }

  inline const std::string &Message() {
    return error_message_;
  }

 private:
  StatusName name_;
  std::string error_message_;
};

// ---------- CHECK ----------

#define CHECK(predicate) {                              \
    if (!(predicate)) {                                 \
      Log(FATAL, L"CHECK failed at %s:%d -> %s",        \
          __FILE__, __LINE__, #predicate);              \
      exit(-1);                                         \
    }                                                   \
  }

void CheckSuccess(Status status, const char *file, int line);

}  // namespace monster_avengers

#define CHECK_SUCCESS(status) {                 \
    CheckSuccess(status, __FILE__, __LINE__);   \
  }


#endif // _MONSTER_AVENGERS_SUPP_HELPERS_H_
