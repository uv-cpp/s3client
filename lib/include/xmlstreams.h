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
/**
 * \file xmlstreams.h
 * \brief xml parser and generator.
 */
#pragma once
#include "s3-api.h"
#include "tinyxml2.h"
#include "xml_path.h"
#include <exception>
#include <string>
#include <variant>
#include <vector>

/// \addtogroup Types
/// @{
/// Representaion of an XML tree as {path to text element, text element} tuples
/// stored into map object. E.g. \code{.xml} <tag1>
///   <tag1_1>
///     text 1 1
///   </tat1_1>
///   <tag1_2>
///     text 1 2
///   </tag1_2>
/// </tag1>
/// \endcode
/// is represented as
/// \code{.cpp}
/// unordered_map<string, string> rec {
///   {"/tag1/tag1_1", "text 1 1"},
///   {"/tag1/tag1_2", "text 1 2"}
/// };
/// \endcode
using XMLRecord = std::unordered_map<std::string, std::string>;
/// Array of XML records
/// \see XMLRecord
using XMLRecords = std::vector<XMLRecord>;

/// Result of an XML query operation
///
/// - Path to text element: return \c std::string
/// - Path to array of text elements: return \c std::vector<std::string>
/// - Path to array of XML subtrees: std::vector<std::unordered_map<std::string,
/// std::string>>
/// - \c false in case element not found, used internally to signal "not found"
/// condition
///
/// \see XMLRecords
using XMLResult =
    std::variant<bool, std::string, std::vector<std::string>, XMLRecords>;
/// @}

/** \addtogroup Parsing
 * @{
 */
/**
 *
 * \brief XMLGenerator. Use methods or overloaded subscript operator to insert
 * data into XML tree.
 *
 * A XML document object and a pointer to the current XML Element are maintained
 * inside the class instance.
 * Elements are added under the current element.
 * The current element pointer can be moved up or down the tree as needed.
 * The generated XML code is extracted by calling the XMLText method.
 *
 * \section Example
 * Generating the ACL request body.
 *
 * Request format:
 * \code{.xml}
 * <AccessControlPolicy>
 *    <Owner>
 *       <DisplayName>string</DisplayName>
 *       <ID>string</ID>
 *    </Owner>
 *    <AccessControlList>
 *       <Grant>
 *          <Grantee>
 *             <DisplayName>string</DisplayName>
 *             <EmailAddress>string</EmailAddress>
 *             <ID>string</ID>
 *             <xsi:type>string</xsi:type>
 *             <URI>string</URI>
 *          </Grantee>
 *          <Permission>string</Permission>
 *       </Grant>
 *    </AccessControlList>
 * </AccessControlPolicy>
 * \endcode
 * Code:
 * \snippet api/xml_parser.cpp GenerateAclXML
 */
class XMLOStream {
public:
  /// Move up or down one level in tree or rewind to root.
  enum MoveAction { UP, DOWN, REWIND };
  /// Move up
  /// \param[in] level number of parents to traverse, default is one
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
  /// Move down after setting text
  /// Set \c down_ flag: pointer to current node will be moved to child after
  /// next insertion
  void Down() { down_ = true; }
  /// Move pointer to current node to root.
  void Rewind() {
    if (!cur_) {
      return;
    }
    for (; cur_->Parent(); cur_ = cur_->Parent()->ToElement())
      ;
  }
  /// Insert text inside current element.
  /// \param[in] text text to insert
  /// \return reference to current \c XMLOStream instance, will return
  /// without inserting in case current pointer to XML element is \c NULL
  XMLOStream &InsertText(const std::string &text) {
    if (!cur_)
      return *this;
    cur_->SetText(text.c_str());
    return *this;
  }
  /// Insert XML element under current XML element.
  /// \param[in] s tag name
  /// \return reference to current \c XMLOStream instance
  XMLOStream &Insert(const std::string &s) {
    std::vector<std::string> elems;
    std::istringstream is(s);
    for (std::string buf; std::getline(is, buf, '/');) {
      if (!buf.empty()) {
        elems.push_back(buf);
      }
    }
    if (elems.empty())
      return *this;
    auto pe = cur_;
    if (!cur_) {
      cur_ = doc_.NewElement(elems.front().c_str());
      doc_.InsertFirstChild(cur_);
      pe = cur_;
    } else {
      pe = cur_->InsertNewChildElement(s.c_str());
    }
    for (auto e = ++begin(elems); e != end(elems); ++e) {
      pe = pe->InsertNewChildElement(e->c_str());
    }
    if (down_) {
      cur_ = pe;
      down_ = false;
    }
    return *this;
  }
  /// Move pointer to XML element.
  /// \see MoveAction
  /// In the case of \c REWIND or \c UP the action is executed immediately
  /// in case of \c DOWN it is executed after next insertion operation
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

  /// Invokes insert method forcing \c DOWN action after insert.
  /// In case only path separator (\c '/') elements are provided the \c UP
  /// action is invoked as many times as there are \c '/' caracters in the
  /// string.
  XMLOStream &operator[](const std::string &s) {
    if (all_of(begin(s), end(s), [](auto c) { return c == '/'; })) {
      Up(s.size());
      return *this;
    }
    down_ = true;
    return Insert(s);
  }
  /// Invoke \c Insert(MoveAction) method
  XMLOStream &operator[](MoveAction a) { return Move(a); }
  /// Constructor: requires a reference to a pre-existing XML document.
  /// \param[in] d wrapped XML document
  XMLOStream(tinyxml2::XMLDocument &d) : doc_(d) {}
  /// Add text element to current XML element and move up one level.
  /// \param[in] s text to insert
  /// \return reference to XMLOStream instance
  XMLOStream &operator=(const std::string &s) {
    InsertText(s);
    Up();
    return *this;
  }
  /// \return current content of XML document as XML text
  std::string XMLText() const { return XMLToText(doc_, true, 0, 0); }
  /// \return reference to current document
  const tinyxml2::XMLDocument &GetDocument() const { return doc_; }

private:
  tinyxml2::XMLDocument &doc_;
  tinyxml2::XMLElement *cur_ = nullptr;
  bool down_ = false;
};

//-----------------------------------------------------------------------------
/** \brief XML parser, returns array of text elements with the same parent path
 *  or an array of parsed XML sub trees.
 *
 *  An XML subtree is represented as record in the form
 *  \code
 *  map<path to text element, text element>
 *  \endcode
 *  under the specifed path.
 *
 * This class stores data of different types inside a \c variant object and
 * extracts the value when the class instance is converted to a type during
 * assignment operations.
 *
 * Since the returned types all have empty values, if the path or text is not
 * found an empty \c std::string, \c std::vector or \c std::unordered_map
 * is returned.
 *
 * \see XMLResult
 *
 */
class XMLIStream {
public:
  /// Extract array of text elements under specified path or
  /// list of records in the form `map<path to text element, text element>`.
  ///
  /// \param[in] p path to text elements if path starts with \c '/' or
  /// to sbtrees under path if it does not start with \c '/'
  ///
  /// \return reference to current instance
  /// \see RecordList
  const XMLIStream &operator[](const std::string &p) const {
    // return all text elements under path
    if (p.front() == '/') {
      auto r = dd_.find(p);
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
      auto r = RecordList("/" + p, dd_);
      if (r.empty()) {
        v_ = false;
      } else {
        v_ = r;
      }
    }
    return *this;
  }
  /// \return current element text or empty string
  operator std::string() const {
    if (auto p = std::get_if<std::string>(&v_)) {
      return *p;
    } else {
      return "";
    }
  }
  /// \return array of text elements or empty array
  operator std::vector<std::string>() const {
    if (auto p = std::get_if<std::vector<std::string>>(&v_)) {
      return *p;
    } else {
      return {};
    }
  }
  /// \return array of parsed records or empty array
  operator std::vector<XMLRecord>() const {
    if (auto p = std::get_if<std::vector<XMLRecord>>(&v_)) {
      return *p;
    } else {
      return {};
    }
  }
  /// Constructor
  /// \param[in] xml xml text
  XMLIStream(const std::string &xml) : dd_(DOMToDict(xml)) {}

private:
  mutable XMLResult v_;
  std::unordered_map<std::string, std::vector<std::string>> dd_;
};

//-----------------------------------------------------------------------------
/// Extract string from map
/// \param[in] r XMLRecord instance \see XMLRecord
/// \param[in] path key in the form "/tag1/tag11..."
/// \return text or empty string if text not found or empty
inline std::string Get(const XMLRecord &r, const std::string &path) {
  auto i = r.find(path);
  return i == end(r) ? std::string() : i->second;
}
/**
 * @}
 */
