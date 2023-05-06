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
#include "xml_path.h"
#include <iostream>
#include <sstream>
#include <vector>
//
#include <algorithm>
#include <iterator>
using namespace std;
using namespace tinyxml2;

template <typename T> void Print(const vector<T> &v) {
  copy(begin(v), end(v), ostream_iterator<T>(cout, ", "));
}

string ToLower(const string &s) {
  string ret;
  for (auto c : s) {
    ret += string::value_type(tolower(c));
  }
  return ret;
}

//-----------------------------------------------------------------------------
const char *cbegin(const char *pc) { return pc; }
const char *cend(const char *pc) {
  while (*pc++ != '\0')
    ;
  return --pc;
}

string Trim(const string &text) {
  auto b = cbegin(text);
  while (isspace(*b) && (b++ != cend(text)))
    ;
  auto e = --cend(text);
  while (isspace(*e) && (e-- != cbegin(text)))
    ;
  return string(b, ++e);
}

//-----------------------------------------------------------------------------
vector<string> ParsePath(const string &path) {
  istringstream is(path);
  vector<string> ret;
  for (string s; getline(is, s, '/');) {
    if (s.empty())
      continue;
    ret.push_back(s);
  }
  return ret;
}

//-----------------------------------------------------------------------------
class FindVisitor : public XMLVisitor {
public:
  bool VisitEnter(const XMLDocument &) { return true; }
  bool VisitExit(const XMLDocument &) { return true; }
  bool VisitEnter(const XMLElement &e, const XMLAttribute *) {
    bool match =
        caseInsesitive_ ? ToLower(e.Name()) == element_ : e.Name() == element_;
    if (match) {
      pElement_ = &e;
      return false;
    }
    return true;
  }
  bool VisitExit(const XMLElement &) {
    if (pElement_)
      return false;
    return true;
  }
  bool Visit(const XMLDeclaration &) { return true; }
  bool Visit(const XMLText &) { return true; }
  bool Visit(const XMLComment &) { return true; }
  bool Visit(const XMLUnknown &) { return true; }

  const XMLElement *GetElement() const { return pElement_; }
  FindVisitor(const string &element, bool caseInsesitive = true)
      : element_(caseInsesitive ? ToLower(element) : element),
        caseInsesitive_(caseInsesitive) {}

private:
  string element_;
  const XMLElement *pElement_ = nullptr;
  bool caseInsesitive_ = true;
};
//-----------------------------------------------------------------------------
class PathVisitor : public XMLVisitor {
public:
  bool VisitEnter(const XMLDocument &) { return true; }
  bool VisitExit(const XMLDocument &) { return true; }
  bool VisitEnter(const XMLElement &e, const XMLAttribute *) {
    curPath_.push_back(caseInsesitive_ ? ToLower(e.Name()) : e.Name());
    if (curPath_ == path_) {
      pElement_ = &e;
      found_ = true;
      return false;
    }
    return true;
  }
  bool VisitExit(const XMLElement &) {
    if (found_)
      return false;
    curPath_.pop_back();
    return true;
  }
  bool Visit(const XMLDeclaration &) { return true; }
  bool Visit(const XMLText &) { return true; }
  bool Visit(const XMLComment &) { return true; }
  bool Visit(const XMLUnknown &) { return true; }

  const XMLElement *GetElement() const { return pElement_; }
  PathVisitor(const string &path, bool caseInsesitive = true)
      : path_(ParsePath(caseInsesitive ? ToLower(path) : path)),
        caseInsesitive_(caseInsesitive) {}
  PathVisitor(const vector<string> &path, bool caseInsesitive = true)
      : caseInsesitive_(caseInsesitive) {
    if (caseInsesitive_) {
      for (const auto &i : path) {
        path_.push_back(ToLower(i));
      }
    } else {
      path_ = path;
    }
  }

private:
  vector<string> path_;
  vector<string> curPath_;
  const XMLElement *pElement_ = nullptr;
  bool found_ = false;
  bool caseInsesitive_ = true;
};
//-----------------------------------------------------------------------------
class MultiPathVisitor : public XMLVisitor {
public:
  bool VisitEnter(const XMLDocument &) { return true; }
  bool VisitExit(const XMLDocument &) { return true; }
  bool VisitEnter(const XMLElement &e, const XMLAttribute *) {
    curPath_.push_back(caseInsensitive_ ? ToLower(e.Name()) : e.Name());
    if (curPath_ == path_) {
      // iterate over children with name == element_
      auto pe = e.FirstChildElement(element_.c_str());
      if (!pe) {
        terminate_ = true;
        return false;
      }
      elements_.push_back(pe);
      while ((pe = pe->NextSiblingElement(element_.c_str()))) {
        elements_.push_back(pe);
      }
      return false;
    }
    return true;
  }
  bool VisitExit(const XMLElement &) {
    if (terminate_)
      return false;
    curPath_.pop_back();
    return true;
  }
  bool Visit(const XMLDeclaration &) { return true; }
  bool Visit(const XMLText &) { return true; }
  bool Visit(const XMLComment &) { return true; }
  bool Visit(const XMLUnknown &) { return true; }

  const vector<const XMLElement *> &GetElements() const { return elements_; }
  MultiPathVisitor(const string &path,
                   const string &element /*child element below path*/,
                   bool caseInsensitive = true)
      : path_(ParsePath(caseInsensitive ? ToLower(path) : path)),
        element_(caseInsensitive ? ToLower(element) : element),
        caseInsensitive_(caseInsensitive) {}
  const XMLElement *operator[](size_t i) { return elements_[i]; }

private:
  vector<string> path_;
  vector<string> curPath_;
  vector<const XMLElement *> elements_;
  bool terminate_ = false;
  string element_;
  bool caseInsensitive_ = true;
};

//-----------------------------------------------------------------------------
string GetElementText(const XMLDocument &doc, const string &path) {
  PathVisitor p(path);
  doc.Accept(&p);
  auto e = p.GetElement();
  if (e && e->GetText()) {
    return Trim(e->GetText());
  } else {
    return "";
  }
}

//-----------------------------------------------------------------------------
string FindElementText(const XMLDocument &doc, const string &element) {
  FindVisitor p(element);
  doc.Accept(&p);
  auto e = p.GetElement();
  if (e && e->GetText()) {
    return Trim(e->GetText());
  } else {
    return "";
  }
}
//-----------------------------------------------------------------------------
string GetElementText(const XMLElement &element, const string &path) {
  PathVisitor p(path);
  element.Accept(&p);
  auto e = p.GetElement();
  if (e && e->GetText()) {
    return Trim(e->GetText());
  } else {
    return "";
  }
}

//-----------------------------------------------------------------------------
vector<const XMLAttribute *> GetAttributes(const XMLElement *e) {
  vector<const XMLAttribute *> as;
  auto a = e->FirstAttribute();
  if (!a) {
    return as;
  }
  as.push_back(a);
  while ((a = a->Next())) {
    as.push_back(a);
  }
  return as;
}

//-----------------------------------------------------------------------------
vector<pair<string, string>> GetAttributesText(const XMLElement *e) {
  vector<pair<string, string>> as;
  auto a = e->FirstAttribute();
  if (!a) {
    return as;
  }
  as.push_back({a->Name(), a->Value()});
  while ((a = a->Next())) {
    as.push_back({a->Name(), a->Value()});
  }
  return as;
}

//-----------------------------------------------------------------------------
string FindElementText(const string &xml, const string &element) {
  tinyxml2::XMLDocument doc;
  if (doc.Parse(xml.c_str()) != tinyxml2::XML_SUCCESS) {
    cerr << "Error parsing XML text" << endl;
    exit(EXIT_FAILURE);
  }
  return Trim(FindElementText(doc, element));
}
//-----------------------------------------------------------------------------
string ParseXMLPath(const string &xml, const string &path) {
  tinyxml2::XMLDocument doc;
  if (doc.Parse(xml.c_str()) != tinyxml2::XML_SUCCESS) {
    cerr << "Error parsing XML text" << endl;
    exit(EXIT_FAILURE);
  }
  return Trim(GetElementText(doc, path));
}

//-----------------------------------------------------------------------------
vector<const XMLElement *> ParseXMLMultiPath(const string &xml,
                                             const string &path,
                                             const string &childEement) {
  tinyxml2::XMLDocument doc;
  if (doc.Parse(xml.c_str()) != tinyxml2::XML_SUCCESS) {
    cerr << "Error parsing XML text" << endl;
    exit(EXIT_FAILURE);
  }
  MultiPathVisitor v(path, childEement);
  doc.Accept(&v);
  return v.GetElements();
}
//-----------------------------------------------------------------------------
vector<string> ParseXMLMultiPathText(const string &xml, const string &path,
                                     const string &childPath) {
  tinyxml2::XMLDocument doc;
  if (doc.Parse(xml.c_str()) != tinyxml2::XML_SUCCESS) {
    cerr << "Error parsing XML text" << endl;
    exit(EXIT_FAILURE);
  }
  auto p = ParsePath(childPath);
  auto head = p.front();
  vector<string> tail(++p.begin(), p.end());
  MultiPathVisitor v(path, head);
  doc.Accept(&v);
  vector<string> ret;
  for (auto i : v.GetElements()) {
    ret.push_back(GetElementText(*i, childPath));
  }
  return ret;
}
