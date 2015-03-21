#ifndef _MONSTER_AVENGERS_HELPERS_H_
#define _MONSTER_AVENGERS_HELPERS_H_

#include <cstdio>
#include <cwchar>

enum LogLevel {
  INFO = 0,
  OK = 1,
  WARNING = 2,
  ERROR = 3,
};

void Log(LogLevel level, const wchar_t *format, ...) {
  fwprintf(stderr, L"%c[%dm", 27, 0);

  switch (level) {
  case ERROR: 
    fwprintf(stderr, L"%c[%d;%dm[Fail] %c[%dm", 27, 1, 31, 27, 0);
    break;
  case WARNING:
    fwprintf(stderr, L"%c[%d;%dm[Warn] %c[%dm", 27, 0, 33, 27, 0);
    break;
  case OK:
    fwprintf(stderr, L"%c[%d;%dm[ OK ] %c[%dm", 27, 0, 32, 27, 0);
    break;
  case INFO:
    fwprintf(stderr, L"[Info] ");
  }
    
  va_list argptr;
  va_start(argptr, format);
  vfwprintf(stderr, format, argptr);
  va_end(argptr);
  fwprintf(stderr, L"%c[%dm\n", 27, 0);
}

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
      Log(ERROR, L"CHECK failed at %s:%d -> %s",        \
          __FILE__, __LINE__, #predicate);              \
      exit(-1);                                         \
    }                                                   \
  }

void CheckSuccess(Status status, const char *file, int line) {
  if (!status.Success()) {
    Log(ERROR, L"CHECK_SUCCESS failed at %s:%d.", file, line);
    Log(ERROR, L"%s", status.message().c_str());
    exit(-1);
  }
}

#define CHECK_SUCCESS(status) {                 \
    CheckSuccess(status, __FILE__, __LINE__);   \
  }


#endif // _MONSTER_AVENGERS_HELPERS_H_
