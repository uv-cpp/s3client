#pragma once
#include <string>
#include <utility>

inline std::string ToLower(const std::string &s) {
  std::string ret;
  for (auto c : s) {
    ret += std::string::value_type(tolower(c));
  }
  return ret;
}

//-----------------------------------------------------------------------------
// trim anything including c std::strings
inline const char *cbegin(const char *pc) { return pc; }
inline const char *cend(const char *pc) {
  while (*pc++ != '\0')
    ;
  return --pc;
}
template <typename IterT> std::pair<IterT, IterT> Trim(IterT begin, IterT end) {
  auto b = begin;
  while (isspace(*b) && (b++ != end))
    ;
  auto e = --end;
  while (isspace(*e) && (e-- != begin))
    ;
  return {b, ++e};
}

// trim c++ std::string
inline std::string Trim(const std::string &text) {
  auto b = cbegin(text);
  while (isspace(*b) && (b++ != cend(text)))
    ;
  auto e = --cend(text);
  while (isspace(*e) && (e-- != cbegin(text)))
    ;
  return std::string(b, ++e);
}
