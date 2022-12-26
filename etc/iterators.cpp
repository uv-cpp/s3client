#include <iostream>
#include <ranges>
#include <string>
#include <string_view>
/// Iterate over splits.
///
/// A constanst reference to the parsed string is kept internally.
/// The delimiter is copied to allow for
class SplitIterator {
  friend class SplitRange;

public:
  using iterator_category = std::forward_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = std::string;
  using pointer = std::string *;   // or also value_type*
  using reference = std::string &; // or also value_type&
public:
  SplitIterator(const std::string &str, const std::string &delims = " ")
      : str_(str), delims_(delims), cur_(str_.find_first_of(delims_)),
        prev_(0) {}

  SplitIterator operator++() {
    if (cur_ == std::string::npos) {
      prev_ = std::string::npos;
      return *this;
    }
    prev_ = cur_ + delims_.size();
    cur_ = str_.find_first_of(delims_, prev_);
    return *this;
  }

  SplitIterator operator++(int) {
    SplitIterator i = *this;
    operator++();
    return i;
  }

  bool operator==(const SplitIterator &other) const {
    return cur_ == other.cur_ && prev_ == other.prev_;
  }

  bool operator!=(const SplitIterator &other) const {
    return !operator==(other);
  }

  std::string operator*() {
    return str_.substr(prev_, std::min(cur_, str_.size()) - prev_);
  }

private:
  const std::string &str_;
  const std::string delims_;
  size_t cur_;
  size_t prev_;
  std::string sub_;
};

class SplitRange {
public:
  SplitRange(const std::string &str, const std::string &delims)
      : str_(str), delims_(delims) {}
  SplitIterator begin() const { return SplitIterator(str_, delims_); }
  SplitIterator end() const {
    auto si = SplitIterator(str_, delims_);
    si.cur_ = std::string::npos;
    si.prev_ = std::string::npos;
    return si;
  }

private:
  const std::string &str_;
  const std::string delims_;
};

inline auto begin(const SplitRange &sr) { return sr.begin(); }

inline auto end(const SplitRange &sr) { return sr.end(); }

using namespace std;

int main(int, char **) {
  const std::string x = "meta1:value1;meta2:value2";
  for (auto i : SplitRange(x, ";")) {
    auto s = begin(SplitRange(i, ":"));
    cout << *s++ << ": " << *s << endl;
  }

  const string_view sv{x};
  const string_view delim{";"};

  for (const auto i : views::split(sv, delim)) {
    cout << i << endl;
  }

  return 0;
}
