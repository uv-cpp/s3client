/*******************************************************************************
 * BSD 3-Clause License
 *
 * Copyright (c) 2020-2023, Ugo Varetto
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions inputFile source code must retain the above copyright
 *    notice, this list inputFile conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list inputFile conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name inputFile the copyright holder nor the names inputFile
 *    its contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/
///@todo move into "detail" namespace

/**
 * \file utility.h
 * \brief internal utility functions
 */

#pragma once

#include <algorithm>
#include <functional>
#include <string>
#include <unordered_map>

/**
 * \addtogroup Internal
 * @{
 */

namespace sss {

/// Split string and put substrings into container
/// \param[in] str input text
/// \param[out] cont container with \c push_back() method
/// \param[in] delims delimiter
/// \param[in] count number of delimiters to parse
template <class ContainerT>
void Split(const std::string &str, ContainerT &cont,
           const std::string &delims = " ",
           const size_t count = std::string::npos) {
  std::size_t cur, prev = 0;
  cur = str.find_first_of(delims);
  size_t i = 0;
  while (cur != std::string::npos && count != i) {
    cont.push_back(str.substr(prev, cur - prev));
    prev = cur + 1;
    cur = str.find_first_of(delims, prev);
    i += 1;
  }
  if (cur == std::string::npos) {
    cont.push_back(str.substr(prev, cur - prev));
  } else {
    cont.push_back(str.substr(prev, std::string::npos));
  }
}

/**
 * \addtogroup Ranges
 * \brief Implementation of ranges functions (ranges are broken in CLang 14-15)
 * https://stackoverflow.com/questions/72716894/how-to-use-stdviewstransform-on-a-vector/72718016
 * @{
 */

/// \brief Iterator over splits.
///
/// A constanst reference to the parsed string is kept internally.
class SplitIterator {
  friend class SplitRange;

public:
  using iterator_category = std::forward_iterator_tag;
  using value_type = std::string;
  using reference = std::string &; // or also value_type&
public:
  SplitIterator(const std::string &str, const std::string &delims = " ")
      : str_(str), delims_(delims), cur_(str_.find_first_of(delims_)),
        prev_(0) {}

  SplitIterator &operator++() {
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
    ++*this;
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

  std::string operator->() { return operator*(); }

private:
  const std::string &str_;
  const std::string delims_;
  size_t cur_;
  size_t prev_;
  std::string sub_;
};

/**
 * \brief Range over splits.
 *
 * \code{.cpp}
 * const std::string x = "meta1:value1;meta2:value2";
 * for (auto i : SplitRange(x, ";")) {
 *   auto s = begin(SplitRange(i, ":"));
 *   cout << *s++ << ": " << *s << endl;
 * }
 * \endcode
 */
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

/// \brief return iterator at start position
inline auto begin(const SplitRange &sr) { return sr.begin(); }
/// \brief return iterator at end position
inline auto end(const SplitRange &sr) { return sr.end(); }
/**
 * @}
 */
/// Return integer in [low,high] range
/// \param lowerBound lower bound
/// \param upperBound upper bound
/// \return random value in interval [\c lowerBound, \c upperBound]
int RandomIndex(int lowerBound, int upperBound);

/// Return file size
/// \param filename file name
/// \return file size in number of bytes
size_t FileSize(const std::string &filename);

/**
 *  \addtogroup Types
 *  @{
 */
using Dict = std::unordered_map<std::string, std::string>;
using Toml = std::unordered_map<std::string, Dict>;
/**
 * @}
 */

/// Parse \e Toml file in AWS format and return tree as map of maps
/// \ingroup Parse
/// Works with AWS format (nested s'=')
/// Parent key is added added to child key: <parent key>/<child_key>'
/// \param filename location of configuration file
/// \return \e Toml content
Toml ParseTomlFile(const std::string &filename);

/// Return home directory
/// \return home directory path
std::string GetHomeDir();

/// Remove leading '#' and leading and trailing blanks in place
/// \ingroup Utility
void TrimLine(std::string &s);

/// Convert to uppercase text
/// \ingroup Utility
/// \param s text
/// \return uppercase text
std::string ToUpper(std::string s);

/// Convert to lowercase
/// \ingroup Utility
/// \param s text
/// \return lowercase text
std::string ToLower(std::string s);
} // namespace sss

/**
 * @}
 *
 */
