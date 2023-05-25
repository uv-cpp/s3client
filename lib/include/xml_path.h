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
/// \brief Return text inside XML tags.
///
/// \param[in] xml XML text.
/// \param[in] element name of XML tag to search for.
/// \return XML text element.
std::string FindElementText(const std::string &xml, const std::string &element);
/// \brief Extract text under XML path.
///
/// \param[in] XML text.
/// \param[in] path path to XML tag in the format "/tag1/tag11/tag112...".
/// \return XML text element.
std::string ParseXMLPath(const std::string &xml, const std::string &path);
/// \brief Multilevel parsing function: return all elements matching path under
/// prefix.
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
/// \param[in] xml XML text.
///
/// \param[in] prefixPath starting path from element extraction.
///
/// \param[in] suffixPath return all text elements matching
///             suffixPath path under \c prefixPath.
///
/// \return list of extracted text elements under \c /prefixPath/suffixPath/
std::vector<std::string> ParseXMLMultiPathText(const std::string &xml,
                                               const std::string &prefixPpath,
                                               const std::string &suffixPath);

/** \brief Extract records from DOM Map. \see DOMToDict.
 * A record is defined as a map with:
 *   - key = path to tag element containing text element in the format
 *   "/tag1/tag2/..."
 *   - value = text element under \a key path
 *
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
 * Transpose operation:
 *
 * FROM:
 * \code
 * /bucket/creation/date  date1
 *                        date2
 *                        date3
 * /bucket/name   name1
 *                name2
 * \endcode
 *
 * TO:
 * \code
 * /bucket/creation/date  date1
 * /bucket/name           name1
 *
 * /bucket/creation/date  date2
 * /bucket/name           name2
 *
 * /bucket/creation/date  date3
 * /bucket/name           ""
 * \endcode
 *
 * \param[in] prefix XML path to record data.
 *
 * \param[in] domMap XML document transformed into {"path", "text"} map through
 *        DOMToDict function.
 *
 * \return list of record maps, each map contains the {field name, field value}
 *         entries for one record.
 *
 */
std::vector<std::unordered_map<std::string, std::string>> RecordList(
    const std::string &prefix,
    const std::unordered_map<std::string, std::vector<std::string>> &domMap);

/// \brief Return all elements at location grouped by element name
///
/// \b Input
/// \code{.xml}
/// <tag1>
///   <tag11>
///     <node1>
///       Text1
///     </node1>
///     <node1>
///       Text1_2
///     </node1>
///     <node2>
///       Text2
///     </node2>
///     <node2>
///       Text2_2
///     </node2>
///   <tag11>
/// </tag1>
/// \endcode
///
/// \b Output
/// \code{.cpp}
///   unordered_map<string, vector<string>> elementsMap = {
///     {"node1", {"Text1", "Text1_2"}},
///     {"node2", {"Text2", "Text2_2"}}
///   };
/// \endcode
///
/// \param[in] xml XML text
/// \param[in] path path to elements: \c "/tag1/tag12/...."
/// \return element text at path grouped by element name
std::unordered_map<std::string, std::vector<std::string>>
ParseXMLPathElementsText(const std::string &xml, const std::string &path);

/// \brief Convert XML text to {path, text element array} map.
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
/// \param[in] xml XML text
/// \return map of {path, element array} where the key
///         is the path to the text elements stored in the value
std::unordered_map<std::string, std::vector<std::string>>
DOMToDict(const std::string &xml);

/// \brief Return map of {subpaths with same prefix, {list of text elements
/// under subpaths}}
///
/// \b Input
/// \code{.cpp}
/// string path = "/path/to/some"
/// unordered_map<string, vector<string>> domMapInput = {
///   {"/path/to/some/textA", {"text A1", "text A2"}},
///   {"/path/to/some_other/text", {"other text"}},
///   {"/path/to/some/textB", {"text B1", "text B2", "text B3"}}
/// };
/// auto subPaths = ExtractSubPaths(path, domMapInput);
/// \endcode
/// \b Output
/// \code{.cpp}
/// unordered_map<string, vector<string>> subPaths = {
///   {"/textA", {"text A1", "text A2"}},
///   {"/textB", {"text B1", "text B2"}}
/// };
/// \endcode
///
/// \param[in] prefix XML prefix in path format: "/.../..."
/// \param[in] domMap DOM document in {prefix => text value} format
/// \return map of subpaths => text element list having prefix \c prefix
std::unordered_map<std::string, std::vector<std::string>> ExtractSubPaths(
    const std::string &prefix,
    const std::unordered_map<std::string, std::vector<std::string>> &domMap);

/// \brief Transform DOM into text, replacing matching keywords with values
/// specified in {keyword => value} map.
///
/// \param[in] doc XML document in \c tinyxml2 format
///
/// \param[in] kv keyword => value map, keywords are replaced with values in
/// translated text
///
/// \param[in] header if \c true addx the \c <xml...> header
///
/// \param[in] eol end of line separator, default is '\n', if '0' no EOL
/// generated
///
/// \param[in] indent indentation value
///
/// \return XML text with keywords replaced with values specified in input map
std::string XMLToText(const tinyxml2::XMLDocument &doc, bool header = true,
                      char eol_ = '\n', int indent = 2,
                      std::unordered_map<std::string, std::string> kv = {});

/// \brief Create XML tree from path format and place it under
/// passed element.
/// I.e. from {"/tag1/tag2", "text"} to "<tag1><tag2>text</tag2></tag1>"
///
/// \param[in] n pointer to root element
/// \param[in] path textual representation of XML path
/// \param[in] text text element to append to path
tinyxml2::XMLElement *CreatePath(tinyxml2::XMLElement *n,
                                 const std::string &path,
                                 const std::string &text = "");

/// \brief Create XML tree from path format and place inside document instance.
tinyxml2::XMLElement *CreatePath(tinyxml2::XMLDocument &doc,
                                 const std::string &path,
                                 const std::string &text = "");

/// \brief Create multiple XML trees under element. Invokes CreatePath multiple
/// times passing {path, text} value at each invocation.
///
/// \param[in] n root element
/// \param[in] path XML path in "/.../..." format
/// \param[in] paths list of {path, text element value} to pass to CreatePath
/// \return XML tree
tinyxml2::XMLElement *
CreatePaths(tinyxml2::XMLElement *n, const std::string &path,
            const std::vector<std::pair<std::string, std::string>> &paths);

/// \brief Create multiple XML trees inside \c tynyxml2 XML document.
/// Invokes CreatePath multiple times passing {path, text} value at each
/// invocation.
///
/// \param[in] n root element
/// \param[in] path XML path in "/.../..." format
/// \param[in] paths list of {path, text element value} to pass to CreatePath
/// \return XML tree
tinyxml2::XMLElement *
CreatePaths(tinyxml2::XMLDocument &doc, const std::string &path,
            const std::vector<std::pair<std::string, std::string>> &paths);
/**
 * @}
 */
