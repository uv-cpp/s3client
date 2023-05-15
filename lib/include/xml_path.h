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

/** Extract records from DOM Map. \see DOMToDict.
 * A record is defined as a map with:
 *   - key = path to tag element containing text element in the format
 *   "/tag1/tag2/..."
 *   - value = text element under \a key path
 *
 * \b Example
 *
 * \b Input:
 *   - \b \c domMap = XML text -> DOMToDict ->
 *     {"/listbucketresult/contents/key", "Key1"}...
 *   - \b \c prefixPath = "/listbucketresult/contents"
 *
 *  \code
 *  <?xml version="1.0" encoding="UTF-8"?>
 *  <ListBucketResult
 *  	xmlns="http://s3.amazonaws.com/doc/2006-03-01/">
 *  	<Name>tst</Name>
 *  	<Prefix></Prefix>
 *  	<MaxKeys>1000</MaxKeys>
 *  	<IsTruncated>false</IsTruncated>
 *  	<Contents>
 *  		<Key>Key1</Key1>
 *  		<Size>67108864</Size>
 *  		<StorageClass>STANDARD</StorageClass>
 *  		<Owner>
 *  			<ID>Owner1</ID>
 *  			<DisplayName>Owner One</DisplayName>
 *  		</Owner>
 *  		<Type>Normal</Type>
 *  	</Contents>
 *  	<Contents>
 *  		<Key>Key2</Key>
 *  		<Size>4294967296</Size>
 *  		<StorageClass>STANDARD</StorageClass>
 *  		<Owner>
 *  			<ID>Owner1</ID>
 *  			<DisplayName>Owner One</DisplayName>
 *  		</Owner>
 *  		<Type>Normal</Type>
 *  	</Contents>
 *  </ListBucketResult>
 *  \endcode
 *
 * \b Output
 *
 * If the case insensitive flag is set (default) then all tag names
 * are converted to lowercase.
 *
 * \code{.cpp}
 * vector<unordered_map<string, string>> record = {
 *     {
 *       {"/key", "Key1"},
 *       {"/size","67108864"},
 *       {"/storageclass", "STANDARD"},
 *       {"/owner/ID", "Owner1"},
 *       {"/owner/displayname", "Owner One"}
 *       {"/type", "Normal"}
 *     },
 *     {
 *       {"/key", "Key2"},
 *       {"/size","4294967296"},
 *       {"/storageclass", "STANDARD"},
 *       {"/owner/ID", "Owner1"},
 *       {"/owner/displayname", "Owner One"}
 *       {"/type", "Normal"}
 *     }
 *   };
 * \endcode
 *
 * \param prefix XML path to record data.
 *
 * \param domMap XML document transformed into {"path", "text"} map through
 *        DOMToDict function.
 *
 * \return list of record maps, each map contains the {field name, field value}
 *         entries for one record.
 *
 */
std::vector<std::unordered_map<std::string, std::string>> RecordList(
    const std::string &prefix,
    const std::unordered_map<std::string, std::vector<std::string>> &domMap);

std::unordered_map<std::string, std::vector<std::string>>
ParseXMLPathElementsText(const std::string &xml, const std::string &path);

/// Convert XML text to {path, text element array} map.
/// \b Input
/// \code{.xml}
/// <tag1>
///   <tag2>
///     Text 1_2
///   </tag2>
///   <tag2>
///     Text 2_2
///   </tag2>
///   <othertag>
///     Other text
///   </othertag>
/// </tag1>
/// \endcode
///
/// \b Output
/// \code{.cpp}
///   unordered_map<string, string> domMap = {
///     {"/tag1/tag2", {"Text 1_2", "Text 2_2"},
///     {"/tag1/othertag", {"Other text"}}
///   };
/// \endcode
///
/// \param xml XML text
/// \return map of {path, element array} where the key
///         is the path to the text elements stored in the value
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