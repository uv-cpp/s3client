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
 *******************************************************************************/
/**
 * \file response_parser.h
 * \brief Functions to parse XML and HTTP headers.
 */
#pragma once

#include <regex>
#include <string>
#include <vector>

#include "common.h"

namespace sss {
/**
 * \addtogroup Parsing
 * \brief Response parset function for XML body and HTTP headers.
 * @{
 */
/// Extract and return content of XML tag
/// \param xml XML text
/// \param tag \c <tag> name
/// \return \c <tag> content
std::string XMLTag(const std::string &xml, const std::string &tag);

/// Extract and return content of all XML tags matching word
/// \param xml XML text
/// \param tag \c <tag> name
/// \return \c content of each tag matching name
std::vector<std::string> XMLTags(const std::string &xml,
                                 const std::string &tag);
/// Extract and return content of XML tag matching hierchical path.
/// \param xml XML text
/// \param path \c tag path separated by \c '/' character
/// \return content of tag if found, empty string otherwise
std::string XMLTagPath(const std::string &xml, const std::string &path);

/// Extract and return HTTP header
/// \param headers text containing the header section of an HTTP payload
/// \param header header name
std::string HTTPHeader(const std::string &headers, const std::string &header);

/// Extract and return HTTP headers as key-value pairs
/// \param headers text containing the header section of an HTTP payload
/// \return \c std::map<std::string, std::string> of {header name, header
/// value} tuples
Headers HTTPHeaders(const std::string &headers);

/// Extract and return \c x-amz-meta-* headers
/// \param headers text containing the header section of an HTTP payload
/// \return \c std::map<std::string, std::string> of {header name, header
/// value} tuples
MetaDataMap MetaDataHeaders(const std::string &headers);

/// Extract ETag string from returned etag text.
/// \param etag etag text
/// \return etag string without quotes
std::string TrimETag(const std::string &etag);
/**
 * @}
 */

} // namespace sss
