/*******************************************************************************
 * BSD 3-Clause License
 *
 * Copyright (c) 2020-2023, Ugo Varetto
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
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
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/

#pragma once
#include "s3-api.h"
#include "tinyxml2.h"
#include "xml_path.h"
#include <exception>
#include <string>
#include <variant>
#include <vector>

using XMLRecord = std::unordered_map<std::string, std::string>;
using XMLRecords = std::vector<XMLRecord>;
using XMLResult =
    std::variant<bool, std::string, std::vector<std::string>, XMLRecords>;

//-----------------------------------------------------------------------------
// XML generator
//-----------------------------------------------------------------------------
class XMLOStream {
public:
  enum MoveAction { UP, DOWN, REWIND };
  void Up(int level = 1) {
    if (!cur_) {
      throw std::logic_error("Cannot pop, Null element");
    }
    if (!cur_->Parent()) {
      throw std::logic_error("Cannot pop, Null parent");
    }
    for (; level; cur_ = cur_->Parent()->ToElement(), level--)
      ;
  }
  void Down() { down_ = true; }

  void Rewind() {
    if (!cur_) {
      return;
    }
    for (; cur_->Parent();
         cur_ = cur_->Parent() ? cur_->Parent()->ToElement() : nullptr)
      ;
  }
  XMLOStream &InsertText(const std::string &text) {
    cur_->SetText(text.c_str());
    return *this;
  }
  XMLOStream &Insert(const std::string &s) {
    if (!cur_) {
      cur_ = doc_.NewElement(s.c_str());
      doc_.InsertFirstChild(cur_);
      down_ = false;
    } else {
      auto e = cur_->InsertNewChildElement(s.c_str());
      if (down_) {
        cur_ = e;
        down_ = false;
      }
    }
    return *this;
  }
  XMLOStream &Move(MoveAction a, int level = 1) {
    switch (a) {
    case UP:
      Up(level);
      break;
    case DOWN:
      Down();
      break;
    case REWIND:
      Rewind();
      break;
    default:
      break;
    }
    return *this;
  }

  XMLOStream &operator[](const std::string &s) {
    if (all_of(begin(s), end(s), [](auto c) { return c == '/'; })) {
      Up(s.size());
      return *this;
    }
    down_ = true;
    return Insert(s);
  }
  XMLOStream &operator[](MoveAction a) { return Move(a); }
  XMLOStream(tinyxml2::XMLDocument &d) : doc_(d) {}
  XMLOStream &operator=(const std::string &s) {
    InsertText(s);
    Up();
    return *this;
  }

  operator std::string() { return XMLToText(doc_, true, 0, 0); }

  const tinyxml2::XMLDocument &GetDocument() const { return doc_; }

private:
  tinyxml2::XMLDocument &doc_;
  tinyxml2::XMLElement *cur_ = nullptr;
  bool down_ = false;
};

//-----------------------------------------------------------------------------
// XML Parser
//-----------------------------------------------------------------------------
class XMLIStream {
public:
  const XMLIStream &operator[](const std::string &i) const {
    // return all text elements under path
    if (i.front() == '/') {
      auto r = dd_.find(i);
      if (r == end(dd_)) {
        v_ = false;
        return *this;
      }
      if (r->second.size() == 1) {
        v_ = r->second.front();
      } else {
        v_ = r->second;
      }
      // return all records under path
    } else {
      auto r = RecordList("/" + i, dd_);
      if (r.empty()) {
        v_ = false;
      } else {
        v_ = r;
      }
    }
    return *this;
  }
  operator std::string() const {
    if (auto p = std::get_if<std::string>(&v_)) {
      return *p;
    } else {
      return "";
    }
  }

  operator std::vector<std::string>() const {
    if (auto p = std::get_if<std::vector<std::string>>(&v_)) {
      return *p;
    } else {
      return {};
    }
  }
  operator std::vector<XMLRecord>() const {
    if (auto p = std::get_if<std::vector<XMLRecord>>(&v_)) {
      return *p;
    } else {
      return {};
    }
  }
  XMLIStream(const std::string &xml) : dd_(DOMToDict(xml)) {}

private:
  mutable XMLResult v_;
  std::unordered_map<std::string, std::vector<std::string>> dd_;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline std::string Get(const XMLRecord &r, const std::string &path) {
  auto i = r.find(path);
  return i == end(r) ? std::string() : i->second;
}
