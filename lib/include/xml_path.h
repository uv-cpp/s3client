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
 * \brief High level XML parsing functions using filepath like input.
 */
#pragma once
#include "tinyxml2.h"
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
/**
 * \addtogroup Parsing
 * @{
 */
/// Return text inside XML tags.
///
/// \param[in] xml XML text.
/// \param[in] element name of XML tag to search for.
/// \return XML text element.
std::string FindElementText(const std::string &xml, const std::string &element);
/// Extract text under XML path.
///
/// \param[in] XML text.
/// \param[in] path path to XML tag in the format "/tag1/tag11/tag112...".
/// \return XML text element.
std::string ParseXMLPath(const std::string &xml, const std::string &path);
/// Multilevel parsing function: return all elements matching path under prefix.
///
/// E.g.
///
/// If prefix is "/tag1/tag11" and suffix is "/name" it will extract text from
/// \b all the <name>Name one</name><name>Name two</name>
/// elements inside \c <tag1><tag2>..</tag2></tag1 equivalent to:
///
/// It groups text elements by XML prefix, mapping prefix to multiple text
/// elements.
///
/// \param xml XML text.
///
/// \param prefixPath starting path from element extraction.
///
/// \param suffixPath return all text elements matching
///        suffixPath path under \c prefixPath.
///
/// \return list of extracted text elements under \c /prefixPath/suffixPath/
std::vector<std::string> ParseXMLMultiPathText(const std::string &xml,
                                               const std::string &prefixPpath,
                                               const std::string &suffixPath);

/// Transpose operation: from prefix->{element list}
///
///
///
std::vector<std::unordered_map<std::string, std::string>>
RecordList(const std::string &prefix,
           const std::unordered_map<std::string, std::vector<std::string>> &d);
std::unordered_map<std::string, std::vector<std::string>>
ParseXMLPathElementsText(const std::string &xml, const std::string &path);
std::unordered_map<std::string, std::vector<std::string>>
DOMToDict(const std::string &xml);

std::unordered_map<std::string, std::vector<std::string>> ExtractRecord(
    const std::string &prefix,
    const std::unordered_map<std::string, std::vector<std::string>> &d);

std::string XMLToText(const tinyxml2::XMLDocument &doc,
                      std::unordered_map<std::string, std::string> kv = {},
                      bool header = true, int indent = 2);
tinyxml2::XMLElement *CreatePath(tinyxml2::XMLElement *n,
                                 const std::string &path,
                                 const std::string &text = "");
tinyxml2::XMLElement *CreatePath(tinyxml2::XMLDocument &doc,
                                 const std::string &path,
                                 const std::string &text = "");
tinyxml2::XMLElement *
CreatePaths(tinyxml2::XMLElement *n, const std::string &path,
            const std::vector<std::pair<std::string, std::string>> &paths);
tinyxml2::XMLElement *
CreatePaths(tinyxml2::XMLDocument &doc, const std::string &path,
            const std::vector<std::pair<std::string, std::string>> &paths);
/**
 * @}
 */
