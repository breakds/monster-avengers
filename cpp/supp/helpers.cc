#include "helpers.h"

namespace monster_avengers {
// ---------- Locale ----------

#if _WIN32
const std::locale LOCALE_UTF8 = 
    std::locale(std::locale(), new std::codecvt_utf8_utf16<wchar_t>());
#else
const std::locale LOCALE_UTF8 = std::locale("en_US.UTF-8");
#endif
    
// ---------- Log ----------

void Log(LogLevel level, const wchar_t *format, ...) {
  fwprintf(stderr, L"%c[%dm", 27, 0);

  switch (level) {
    case FATAL: 
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
  fflush(stderr);
}

void CheckSuccess(Status status, const char *file, int line) {
  if (!status.Success()) {
    Log(FATAL, L"CHECK_SUCCESS failed at %s:%d.", file, line);
    Log(FATAL, L"%s", status.message().c_str());
    exit(-1);
  }
}

}  // namespace monster_avengers

