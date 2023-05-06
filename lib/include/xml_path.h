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
#include "tinyxml2.h"
#include <string>
#include <variant>
#include <vector>

std::string ParseXMLPath(const std::string &xml, const std::string &path);
std::vector<std::string> ParseXMLMultiPathText(const std::string &xml,
                                               const std::string &path,
                                               const std::string &childPath);
const tinyxml2::XMLElement *GetElement(tinyxml2::XMLDocument &doc,
                                       const std::string &xml,
                                       std::string path);

const tinyxml2::XMLElement *GetElement(const tinyxml2::XMLElement *element,
                                       std::string path);
std::vector<const tinyxml2::XMLElement *>
GetElements(tinyxml2::XMLDocument &doc, const std::string &xml,
            const std::string &path);
std::string GetElementText(tinyxml2::XMLDocument &doc, const std::string &path);

std::vector<std::pair<std::string, std::string>>
GetElementsText(tinyxml2::XMLDocument &doc, const std::string &xml,
                const std::string &path);

std::vector<const tinyxml2::XMLAttribute *>
GetAttributes(const tinyxml2::XMLElement *e);

std::vector<std::pair<std::string, std::string>>
GetAttributesText(const tinyxml2::XMLElement *e);
