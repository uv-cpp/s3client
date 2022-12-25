/*******************************************************************************
 * BSD 3-Clause License
 *
 * Copyright (c) 2020-2022, Ugo Varetto
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/
///@todo move into "detail" namespace

/**
 * \file url_utility.h
 * \brief Internal utility functions to process and parse text and URLs and
 * create S3 signature keys.
 *
 */

#pragma once
#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "common.h"

/** \addtogroup internal
 *  @{
 */

namespace sss {

/// Split string and puts substrings into container
/// \param str input text
/// \param[out] cont container with \c push_back() method
/// \param delims delimiter
/// \param count number of delimiters to parse
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

// const std::string x = "meta1:value1;meta2:value2";
// for (auto i : SplitRange(x, ";")) {
//   auto s = begin(SplitRange(i, ":"));
//   cout << *s++ << ": " << *s << endl;
// }

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

/// URL type
struct URL {
  int port = -1;     ///< internet port
  std::string host;  ///< host name
  std::string proto; ///< protocol, one of \c http or \c https
};

/// Extract URL parameters from string
/// \sa URL
/// \param s text
/// \return URL
URL ParseURL(const std::string &s);

/// URL-encode url
/// \param s text
/// \return url-encoded url
std::string UrlEncode(const std::string &s);

/// URL-encode url from \c {key,value} pairs
/// \param p \c {key,value} map
/// \return url-encoded url
std::string UrlEncode(const Map &p);

/// Time data type, used to generate pre-signed URLs
struct Time {
  std::string timeStamp; ///< full date-time in \c "%Y%m%dT%H%M%SZ" format
  std::string dateStamp; ///< date in \c "%Y%m%d" format
};

/// Return current time and data in the two formats required to sign AWS S3
/// requests
/// \return Time structure filled with both full date-time and date only
Time GetDates();

/// Unsigned byte array
using Bytes = std::vector<uint8_t>;

/// Create \e HMAC encoded byte array
/// \param key
/// \param msg message to encode
/// \return HMAC hash
Bytes Hash(const Bytes &key, const Bytes &msg);

/// Translate bytes to hexadecimal encoded ASCII string
/// \param b byte array
/// \return ASCII-encoded hex numbers
std::string Hex(const Bytes &b);

/// Create signature key
/// \param key this is the secret part of the {key,secret} credentials
/// \param dateStamp date in the format "%Y%m%d"
/// \param region region e.g. \c us-east-1
/// \param service service e.g. \c s3
/// \return hash of internally generated key and binary-encoded text \c
/// "aws4_request"
Bytes CreateSignatureKey(const std::string &key, const std::string &dateStamp,
                         const std::string &region, const std::string &service);

/// Convert to uppercase text
/// \param s text
/// \return uppercase text
std::string ToUpper(std::string s);

/// Convert to lowercase
/// \param s text
/// \return lowercase text
std::string ToLower(std::string s);

/// From \c "key1=value1;key2=value2;key3=;key4" to \c {key, value} dictionary
/// \param s text
/// \return \c {name,value} dictionary
Parameters ParseParams(std::string s);

/// From \c "key1:value1;key2:value2 to \c {key, value} dictionary
/// See \c https://www.w3.org/Protocols/rfc2616/rfc2616.html
Headers ParseHeaders(const std::string &s);

/// Adds \c x-amz-meta- prefix to map keys and checks that total size is less
/// than maximum metadata header size (currently 2kB).
Map ToMeta(const Map &metadata);

} // namespace sss

/**
 * @}
 */
