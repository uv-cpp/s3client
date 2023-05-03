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
using namespace std;
using namespace tinyxml2;
//-----------------------------------------------------------------------------
const XMLElement *GetElement(XMLDocument &doc, const string &xml, string path) {
  if (doc.Parse(xml.c_str()) != XML_SUCCESS) {
    return nullptr;
  }
  const XMLElement *e = nullptr;
  string n;
  istringstream is(path);
  while (getline(is, n, '/')) {
    if (n.empty())
      continue;
    if (!e) {
      e = doc.FirstChildElement(n.c_str()); //->FirstChildElement(path.c_str());
    } else {
      e = e->FirstChildElement(n.c_str());
    }
    if (!e)
      return nullptr;
  }
  return e->DeepClone(&doc)->ToElement();
}

//-----------------------------------------------------------------------------
const XMLElement *GetElement(const XMLElement *pe, string path) {
  if (!pe)
    return nullptr;
  const XMLElement *e = nullptr;
  string n;
  istringstream is(path);
  while (getline(is, n, '/')) {
    if (n.empty())
      continue;
    if (!e) {
      if (pe->Name() == n) {
        return pe;
      } else {
        e = pe->FirstChildElement(n.c_str());
      }
    } else {
      e = e->FirstChildElement(n.c_str());
    }
    if (!e)
      return nullptr;
  }
  return e;
}
//-----------------------------------------------------------------------------
vector<const XMLElement *> GetElements(XMLDocument &doc, const string &xml,
                                       const string &path) {
  vector<const XMLElement *> els;
  auto e = GetElement(doc, xml, path);
  if (!e) {
    return els;
  }
  auto c = e->FirstChildElement();
  if (!c) {
    return els;
  }
  els.push_back(c);
  while ((c = c->NextSiblingElement())) {
    els.push_back(c);
  }
  return els;
}

//-----------------------------------------------------------------------------
string GetElementText(XMLDocument &doc, const string &xml, const string &path) {
  auto e = GetElement(doc, xml, path);
  if (!e) {
    return "";
  }
  return e->GetText() ? e->GetText() : "";
}

//-----------------------------------------------------------------------------
vector<pair<string, string>>
GetElementsText(XMLDocument &doc, const string &xml, const string &path) {
  vector<pair<string, string>> els;
  auto e = GetElement(doc, xml, path);
  if (!e) {
    return els;
  }
  auto c = e->FirstChildElement();
  if (!c) {
    return els;
  }
  els.push_back({c->Name(), c->GetText() ? c->GetText() : ""});
  while ((c = c->NextSiblingElement())) {
    els.push_back({c->Name(), c->GetText() ? c->GetText() : ""});
  }
  return els;
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
