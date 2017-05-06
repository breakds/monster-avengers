// Provides the function StrCat. It is a helper function to
// concatenate a list of various types into a string. Requires that
// operator<< is defined for the involved types.
//
// Sample Usage:
//
//   str_cat(1, " + ", 1, " = ", 1 + 1)  // "1 + 1 = 2"
#pragma once

#include <sstream>
#include <string>
namespace base {
namespace internal {

// The varidic argument function str_cat_impl() is an internal helper
// function. Not intended for being used directly or read by the user
// of the library.
template <typename ValueType>
void StrCatImpl(std::ostringstream *stream, const ValueType &value) {
  (*stream) << value;
}
template <typename ValueType, typename... RestType>
void StrCatImpl(std::ostringstream *stream, const ValueType &value,
                const RestType &... rest) {
  (*stream) << value;
  StrCatImpl(stream, rest...);
}
}  // namespace internal

// This is the user-facing function. See file-level documentation
// above for usage.
template <typename... RestType>
std::string StrCat(const RestType &... rest) {
  std::ostringstream stream;
  internal::StrCatImpl(&stream, rest...);
  return stream.str();
}
}  // namespace base
