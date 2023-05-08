/*******************************************************************************
 * BSD 3-Clause License
 *
 * Copyright (c) 2020-2023, Ugo Varetto
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions inputFile source code must retain the above copyright
 *    notice, this list inputFile conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list inputFile conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name inputFile the copyright holder nor the names inputFile
 *    its contributors may be used to endorse or promote products derived from
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

#ifdef __GNUC__
#if __GNUC__ > 8
#include <filesystem>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif
#else
#include <filesystem>
#endif

#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <random>
#include <regex>

#include "utility.h"

namespace sss {

size_t FileSize(const std::string &filename) {
#ifdef __GNUC__
#if __GNUC__ > 8
  return std::filesystem::file_size(filename);
#else
  struct stat st;
  if (stat(filename.c_str(), &st) == 0)
    return st.st_size;
  return 0;
#endif
#else
  return std::filesystem::file_size(filename);
#endif
}

std::string GetHomeDir() {
  struct passwd *pw = getpwuid(getuid());
  return pw->pw_dir;
}

using namespace std;

void TrimLine(string &s) {
  auto i = s.find("#");
  if (i != string::npos)
    s.erase(i);
  i = s.find_last_not_of(" \r\n\t");
  if (i != string::npos)
    s.erase(++i);
}

Toml ParseTomlFile(const string &filename) {
  ifstream is(filename);
  if (!is) {
    throw std::invalid_argument("Cannot open file " + filename);
    return {};
  }
  string line;
  const string section = "^\\s*\\[\\s*([^\\]]+)\\]\\s*$";
  int lineCount = 0;
  string curSection;
  Toml toml;
  Dict curSectionData;
  string lastKey;
  while (getline(is, line)) {
    ++lineCount;
    if (line.size() == 0 || regex_search(line, regex{"^\\s*#"})) {
      continue;
    }
    smatch sm;
    if (regex_search(line, sm, regex{section})) {
      if (curSection.size() != 0) {
        toml[curSection] = curSectionData;
        curSectionData.clear();
        lastKey.clear();
      }
      curSection = sm[1];
    } else {
      smatch sm;
      if (regex_search(line, sm, regex{"\\s*(\\w+)\\s*=\\s*(.*)"})) {
        string value = sm[2];
        TrimLine(value);
        string keyPrefix = "";
        if (!lastKey.empty() && curSectionData[lastKey].empty()) {
          keyPrefix = lastKey + "/";
        }
        curSectionData[keyPrefix + string(sm[1])] = value;
        lastKey = sm[1];
      } else {
        if (lastKey.empty()) {
          throw std::runtime_error("Toml parsing error, line " +
                                   to_string(lineCount));
        }
        if (line.size() == 0)
          continue;
        TrimLine(line);
        curSectionData[lastKey] += line;
      }
    }
  }
  if (toml.find(curSection) == toml.end()) {
    toml[curSection] = curSectionData;
  }

  return toml;
}

int RandomIndex(int lowerBound, int upperBound) {
  std::random_device r;
  std::seed_seq seed{r(), r(), r(), r(), r(), r(), r(), r()};
  std::mt19937 e(seed);
  std::uniform_int_distribution<int> uniformDist(lowerBound, upperBound);
  return uniformDist(e);
}

} // namespace sss
