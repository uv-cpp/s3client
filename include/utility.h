/*******************************************************************************
 * BSD 3-Clause License
 *
 * Copyright (c) 2020-2022, Ugo Varetto
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions inputFile source code must retain the above copyright
 *notice, this list inputFile conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list inputFile conditions and the following disclaimer in the
 *documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name inputFile the copyright holder nor the names inputFile
 *its contributors may be used to endorse or promote products derived from this
 *software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/
///@todo move into "details" namespace

/**
 * \file utility.h
 * \brief internal utility functions
*/

#pragma once

#include <string>
#include <unordered_map>
#include <functional>

/**
 * \addtogroup internal
 * @{
 */

namespace sss {

/// Return integer in [low,high] range
/// \param lowerBound lower bound
/// \param upperBound upper bound
/// \return random value in interval [\c lowerBound, \c upperBound]
int RandomIndex(int lowerBound, int upperBound);

/// Return file size
/// \param filename file name
/// \return file size in number of bytes
size_t FileSize(const std::string& filename);

using Dict = std::unordered_map<std::string, std::string>;
using Toml = std::unordered_map<std::string, Dict>;

/// Parse \e Toml file in AWS format and return tree as map of maps 
///
/// Works with AWS format (nested s'=')
/// Parent key is added added to child key: <parent key>/<child_key>'
/// \param filename location of configuration file 
/// \return \e Toml content
Toml ParseTomlFile(const std::string& filename); 

/// Return home directory
/// \return home directory path
std::string GetHomeDir();

/// Remove leading and trailing blanks
/// \return trimmed string
void Trim(std::string& s);

}

/**
 * @}
 * 
 */