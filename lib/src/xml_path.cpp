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
using namespace std;
using namespace tinyxml2;

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
class PathVisitor : public XMLVisitor {
public:
  bool VisitEnter(const XMLDocument &) { return true; }
  bool VisitExit(const XMLDocument &) { return true; }
  bool VisitEnter(const XMLElement &e, const XMLAttribute *) {
    curPath_.push_back(e.Name());
    if (curPath_ == path_) {
      if (e.GetText()) {
        text_ = e.GetText();
      }
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

  const string &GetText() const { return text_; }
  PathVisitor(const vector<string> &path) : path_(path) {}
  PathVisitor(const string &path) : path_(ParsePath(path)) {}

private:
  vector<string> path_;
  vector<string> curPath_;
  string text_;
  bool found_ = false;
};
//-----------------------------------------------------------------------------
class MultiPathVisitor : public XMLVisitor {
public:
  bool VisitEnter(const XMLDocument &) { return true; }
  bool VisitExit(const XMLDocument &) { return true; }
  bool VisitEnter(const XMLElement &e, const XMLAttribute *) {
    curPath_.push_back(e.Name());
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
                   const string &element /*child element below path*/)
      : path_(ParsePath(path)), element_(element) {}

private:
  vector<string> path_;
  vector<string> curPath_;
  vector<const XMLElement *> elements_;
  bool terminate_ = false;
  string element_;
};

//-----------------------------------------------------------------------------
string GetElementText(XMLDocument &doc, const string &path) {
  PathVisitor p(path);
  doc.Accept(&p);
  return p.GetText();
}

//-----------------------------------------------------------------------------
string GetElementText(XMLElement &element, const string &path) {
  PathVisitor p(path);
  element.Accept(&p);
  return p.GetText();
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
  auto p = ParsePath(path);
  auto head = p.front();
  vector<string> tail(++p.begin(), p.end());
  MultiPathVisitor v(path, head);
  doc.Accept(&v);
  vector<string> ret;
  for (auto i : v.GetElements()) {
    PathVisitor vc(tail);
    i->Accept(&vc);
    ret.push_back(Trim(i->GetText()));
  }
  return ret;
}
