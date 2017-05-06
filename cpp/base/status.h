#pragma once

#include <string>
#include "base/strcat.h"

namespace base {

// Denotes success or failure of an event, i.e. a function call. The
// Status consists of an error code and an error message.
//
// Error code 0 is reserved for OK (Success).
class Status {
 public:
  template <typename... RestType>
  Status(int code, const RestType &... rest)
      : code_(code), message_(StrCat(rest...)) {}

  Status(int code) : Status(code, "") {}

  Status(const Status &other) : Status(other.code_, other.message_) {}

  const Status &operator=(const Status &other) {
    code_ = other.code_;
    message_ = other.message_;
    return *this;
  }

  Status(Status &&other)
      : code_(other.code_), message_(std::move(other.message_)) {}

  const Status &operator=(Status &&other) {
    return Status(std::move(other));
  }

  static Status OK() {
    return Status(0);
  }

  // Methods.

  bool operator==(const Status &other) const {
    return other.code_ == code_ && other.message_ == message_;
  }

  bool operator!=(const Status &other) const {
    return !((*this) == other);
  }

  bool ok() const {
    return code_ == 0;
  }

  int code() const {
    return code_;
  }

  const std::string &error_message() const {
    return message_;
  }

 private:
  int code_;
  std::string message_;
};
}  // namespace base
