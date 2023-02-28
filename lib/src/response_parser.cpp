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
 ******************************************************************************/
#include "response_parser.h"
#include <iostream>
#include <regex>

using namespace std;

namespace sss {

string XMLTag(const string &xml, const string &tag) {
  const regex rx{tag + "\\s*>\\s*([^\\s<]+)\\s*<", regex_constants::icase};
  smatch sm;
  if (!regex_search(xml, sm, rx)) {
    return "";
  }
  return sm[1];
}

vector<string> XMLTags(const string &xml, const string &tag) {
  const regex rx{tag + "\\s*>\\s*([^\\s<]+)\\s*<", regex_constants::icase};
  smatch sm;
  vector<string> tags;
  if (!regex_search(xml, sm, rx)) {
    return tags;
  }
  for (const auto &i : sm) {
    tags.push_back(i.str());
  }
  return tags;
}

string HTTPHeader(const string &headers, const string &header) {
  const regex rx{header + "\\s*:\\s*([^\\s]+)", regex_constants::icase};
  smatch sm;
  if (!regex_search(headers, sm, rx)) {
    return "";
  }
  return sm[1]; // vector<uint8_t> h = req.GetResponseHeader();
                // string hs(begin(h), end(h));
}

namespace {
Headers HTTPHeaderFilter(const string &txt, const string &filterRx) {
  Headers headers;
  regex h(filterRx, regex_constants::icase);
  smatch sm;
  string::const_iterator si(txt.cbegin());
  while (regex_search(si, txt.cend(), sm, h)) {
    if (sm.size() > 1) {
      headers[sm[1]] = sm[2];
    }
    si = sm.suffix().first;
  }
  return headers;
}
} // namespace

Headers HTTPHeaders(const string &txt) {
  return HTTPHeaderFilter(txt, "\\s*(\\w+)\\s*:\\s*(\\w+)");
}

MetaDataMap MetaDataHeaders(const string &txt) {
  return HTTPHeaderFilter(txt, "\\s*x-amz-meta-(\\w+)\\s*:\\s*(\\w+)");
}
} // namespace sss
