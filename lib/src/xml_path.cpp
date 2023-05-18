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
#include "utility.h"
#include <deque>
#include <iostream>
#include <map>
#include <sstream>
#include <unordered_map>
#include <vector>
//
#include <algorithm>
#include <iterator>
using namespace std;
using namespace tinyxml2;

//-----------------------------------------------------------------------------
string ToLower(const string &s) {
  string r;
  for (auto c : s) {
    r += tolower(c);
  }
  return r;
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
std::string Trim(const std::string &text) {
  auto b = cbegin(text);
  while (isspace(*b) && (b++ != cend(text)))
    ;
  auto e = --cend(text);
  while (isspace(*e) && (e-- != cbegin(text)))
    ;
  return std::string(b, ++e);
}

//-----------------------------------------------------------------------------
// Given prefix as "/.../..."
// and DOM in map format: <...></...> ->  "/path" => "text"
// return all sub paths with the same prefix
unordered_map<string, vector<string>>
ExtractSubPaths(const string &prefix,
                const unordered_map<string, vector<string>> &d) {
  unordered_map<string, vector<string>> ret;
  for (const auto &kv : d) {
    if (kv.first.substr(0, prefix.size()) == prefix) {
      auto &v = ret[kv.first.substr(prefix.size())];
      v.insert(v.end(), kv.second.begin(), kv.second.end());
    }
  }
  return ret;
}

//-----------------------------------------------------------------------------
vector<unordered_map<string, string>>
RecordList(const string &prefix,
           const unordered_map<string, vector<string>> &d) {
  auto r = ExtractSubPaths(prefix, d);
  if (r.empty())
    return {};
  size_t maxElements = 0;
  vector<unordered_map<string, string>> ret;
  for (const auto &kv : r) {
    maxElements = max(maxElements, kv.second.size());
  }
  for (size_t i = 0; i != maxElements; ++i) {
    unordered_map<string, string> e;
    for (const auto &kv : r) {
      e.insert({kv.first, i < kv.second.size() ? kv.second[i] : ""});
    }
    ret.push_back(e);
  }
  return ret;
}

template <typename T> void Print(const vector<T> &v) {
  copy(begin(v), end(v), ostream_iterator<T>(cout, ", "));
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
deque<string> ParsePathQueue(const string &path) {
  istringstream is(path);
  deque<string> ret;
  for (string s; getline(is, s, '/');) {
    if (s.empty())
      continue;
    ret.push_back(s);
  }
  return ret;
}

string ToPath(const vector<string> &v) {
  string path;
  for (const auto &i : v) {
    path += "/" + i;
  }
  return path;
}
//-----------------------------------------------------------------------------
// Find first element
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
// return element at specific location
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
// print xml text and replace template keyword with provided map values e.g.
// <tag1>$1</tag1> -> <tag1>Replaced value</tag1>
class ToTextVisitor : public XMLVisitor {
public:
  bool VisitEnter(const XMLDocument &) {
    if (header_) {
      os_ << "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
      if (eol_) {
        os_ << eol_;
      }
    }
    return true;
  }
  bool VisitExit(const XMLDocument &) { return true; }
  bool VisitEnter(const XMLElement &e, const XMLAttribute *) {
    os_ << string(indent_, ' ') << '<' << e.Name();
    if (const XMLAttribute *a = e.FirstAttribute()) {
      os_ << ' ' << a->Name() << '=' << '"' << Map(a->Value()) << '"';
      while ((a = a->Next())) {
        os_ << ' ' << a->Name() << '=' << '"' << Map(a->Value()) << '"';
      }
      os_ << ' ';
    }
    os_ << ">";
    if (eol_) {
      os_ << eol_;
    }
    indent_ += indentIncrement_;
    return true;
  }
  bool VisitExit(const XMLElement &e) {
    indent_ -= indentIncrement_;
    os_ << string(indent_, ' ') << "</" << e.Name() << '>';
    if (eol_) {
      os_ << eol_;
    }
    return true;
  }
  bool Visit(const XMLDeclaration &) { return true; }
  bool Visit(const XMLText &t) {
    indent_ += indentIncrement_;
    os_ << string(indent_, ' ') << Map(t.Value());
    if (eol_) {
      os_ << eol_;
    }
    indent_ -= indentIncrement_;
    return true;
  }
  bool Visit(const XMLComment &) { return true; }
  bool Visit(const XMLUnknown &) { return true; }
  string Text() const { return os_.str(); }
  ToTextVisitor(bool header = true, char eol = '\n', int indentIncrement = 2)
      : header_(header), eol_(eol), indentIncrement_(indentIncrement) {}

  ToTextVisitor(unordered_map<string, string> keys, bool header = true,
                char eol = '\n', int indentIncrement = 2)
      : map_(keys), header_(header), eol_(eol),
        indentIncrement_(indentIncrement) {}

private:
  const string &Map(const string &k) { return map_.empty() ? k : map_.at(k); }

private:
  unordered_map<string, string> map_;
  bool header_ = true;
  int indentIncrement_ = 2;
  int indent_ = 0;
  ostringstream os_;
  bool caseInsesitive_ = true;
  char eol_;
};
//-----------------------------------------------------------------------------
// return all children of path with specific name
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
// build path -> element text unordered_map where the keys are the full path to
// text each unordered_maps to one or more elments
class DOMToDictVisitor : public XMLVisitor {
public:
  bool VisitEnter(const XMLDocument &) { return true; }
  bool VisitExit(const XMLDocument &) { return true; }
  bool VisitEnter(const XMLElement &e, const XMLAttribute *) {
    curPath_.push_back(caseInsensitive_ ? ToLower(e.Name()) : e.Name());
    return true;
  }
  bool VisitExit(const XMLElement &) {
    curPath_.pop_back();
    return true;
  }
  bool Visit(const XMLDeclaration &) { return true; }
  bool Visit(const XMLText &t) {
    dict_[ToPath(curPath_)].push_back(t.Value());
    return true;
  }
  bool Visit(const XMLComment &) { return true; }
  bool Visit(const XMLUnknown &) { return true; }

  DOMToDictVisitor(bool caseInsensitive = true)
      : caseInsensitive_(caseInsensitive) {}

  const unordered_map<string, vector<string>> &GetDict() const { return dict_; }

private:
  vector<string> curPath_;
  bool caseInsensitive_ = true;
  unordered_map<string, vector<string>> dict_;
};
//-----------------------------------------------------------------------------
// return all children of path
class MultiVisitor : public XMLVisitor {
public:
  bool VisitEnter(const XMLDocument &) { return true; }
  bool VisitExit(const XMLDocument &) { return true; }
  bool VisitEnter(const XMLElement &e, const XMLAttribute *) {
    curPath_.push_back(caseInsensitive_ ? ToLower(e.Name()) : e.Name());
    if (curPath_ == path_) {
      // iterate over children with name == element_
      auto pe = e.FirstChildElement();
      if (!pe) {
        terminate_ = true;
        return false;
      }
      elements_.push_back(pe);
      while ((pe = pe->NextSiblingElement())) {
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
  MultiVisitor(const string &path, bool caseInsensitive = true)
      : path_(ParsePath(caseInsensitive ? ToLower(path) : path)),
        caseInsensitive_(caseInsensitive) {}

private:
  vector<string> path_;
  vector<string> curPath_;
  vector<const XMLElement *> elements_;
  bool terminate_ = false;
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
    throw std::logic_error("Error parsing XML text");
  }
  return Trim(FindElementText(doc, element));
}
//-----------------------------------------------------------------------------
// return element text at location
string ParseXMLPath(const string &xml, const string &path) {
  tinyxml2::XMLDocument doc;
  if (doc.Parse(xml.c_str()) != tinyxml2::XML_SUCCESS) {
    throw std::logic_error("Error parsing XML text");
  }
  return Trim(GetElementText(doc, path));
}

//-----------------------------------------------------------------------------
vector<const XMLElement *> ParseXMLMultiPath(const string &xml,
                                             const string &path,
                                             const string &childEement) {
  tinyxml2::XMLDocument doc;
  if (doc.Parse(xml.c_str()) != tinyxml2::XML_SUCCESS) {
    throw std::logic_error("Error parsing XML text");
  }
  MultiPathVisitor v(path, childEement);
  doc.Accept(&v);
  return v.GetElements();
}

//-----------------------------------------------------------------------------
// return text at specific location under prefixr e.g. <tag1>/<tag2> under
// <parent1>/<parent2>
vector<string> ParseXMLMultiPathText(const string &xml, const string &path,
                                     const string &childPath) {
  tinyxml2::XMLDocument doc;
  if (doc.Parse(xml.c_str()) != tinyxml2::XML_SUCCESS) {
    throw std::logic_error("Error parsing XML text");
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

//-----------------------------------------------------------------------------
// return all elements at location grouped by element name
unordered_map<string, vector<string>>
ParseXMLPathElementsText(const string &xml, const string &path) {
  tinyxml2::XMLDocument doc;
  if (doc.Parse(xml.c_str()) != tinyxml2::XML_SUCCESS) {
    throw std::logic_error("Error parsing XML text");
  }
  MultiVisitor v(path);
  doc.Accept(&v);
  unordered_map<string, vector<string>> ret;
  for (auto i : v.GetElements()) {
    string text = i->GetText() ? i->GetText() : "";
    if (!text.empty()) {
      ret[i->Name()].push_back(text);
    }
  }
  return ret;
}

//-----------------------------------------------------------------------------
unordered_map<string, vector<string>> DOMToDict(const string &xml) {
  tinyxml2::XMLDocument doc;
  if (doc.Parse(xml.c_str()) != tinyxml2::XML_SUCCESS) {
    throw std::logic_error("Error parsing XML text");
  }
  DOMToDictVisitor v;
  doc.Accept(&v);
  return v.GetDict();
}

//-----------------------------------------------------------------------------
// XML Generation
XMLElement *CreatePath(XMLElement *e, deque<string> path,
                       const string &text = "") {
  XMLElement *n = e;
  while (!path.empty()) {
    n = n->InsertNewChildElement(path.front().c_str());
    path.pop_front();
  }
  if (!text.empty()) {
    n->SetText(text.c_str());
  }
  return n;
}

XMLElement *CreatePath(XMLElement *n, const string &path, const string &text) {
  auto p = ParsePathQueue(path);
  return CreatePath(n, p, text);
}

XMLElement *CreatePath(XMLDocument &doc, const string &path,
                       const string &text) {
  auto p = ParsePathQueue(path);
  XMLElement *n = doc.NewElement(p.front().c_str());
  p.pop_front();
  return CreatePath(n, p, text);
}

XMLElement *CreatePaths(XMLElement *n, const string &path,
                        const vector<pair<string, string>> &paths) {
  auto e = CreatePath(n, path);
  for (const auto &i : paths) {
    CreatePath(e, i.first, i.second);
  }
  return e;
}

XMLElement *CreatePaths(XMLDocument &doc, const string &path,
                        const vector<pair<string, string>> &paths) {
  auto e = CreatePath(doc, path);
  for (const auto &i : paths) {
    CreatePath(e, i.first, i.second);
  }
  return e;
}

string XMLToText(const XMLDocument &doc, bool header, char eol, int indent,
                 unordered_map<string, string> kv) {
  ToTextVisitor v(std::move(kv), header, eol, indent);
  doc.Accept(&v);
  return v.Text();
}
