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
 *this list of conditions and the following disclaimer.
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
 *ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/
#include "utility.h"
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <stdexcept>
using namespace std;
using namespace sss;
using namespace api;

//----------------------------------------------------------------------------
Params ParseCmdLine(int argc, char **argv) {

  auto printUsage = [](char **argv, ostream &os) {
    cerr << "Usage: " << argv[0] << " <Access environment variable> "
         << "Secret environment variable> <URL environment variable>" << endl;
  };
  if (argc != 4) {
    cerr << "Wrong number of arguments" << endl;
    printUsage(argv, cerr);
    exit(EXIT_FAILURE);
  }
  const string access = getenv(argv[1]) ? getenv(argv[1]) : "";
  if (access.empty()) {
    cerr << "Error: cannot read access env var " << argv[1] << endl;
    exit(EXIT_FAILURE);
  }
  const string secret = getenv(argv[2]) ? getenv(argv[2]) : "";
  if (secret.empty()) {
    cerr << "Error: cannot read secret env var " << argv[2] << endl;
    exit(EXIT_FAILURE);
  }
  const string url = getenv(argv[3]) ? getenv(argv[3]) : "";
  if (secret.empty()) {
    cerr << "Error: cannot read url env var " << argv[3] << endl;
    exit(EXIT_FAILURE);
  }
  return {.access = access, .secret = secret, .url = url};
}

//----------------------------------------------------------------------------
void TestS3Access(const Params &config) {
  try {
    S3Client s3(config.access, config.secret, config.url);
    s3.ListBuckets();
  } catch (const exception &e) {
    cerr << e.what() << endl
         << "Cannot access " << config.url << endl
         << "  access: " << config.access << endl
         << "  secret: " << config.secret << endl;
    exit(EXIT_FAILURE);
  }
}

//----------------------------------------------------------------------------
void TestOutput(const std::string &name, bool success,
                const std::string &prefix, const std::string &msg, bool eol) {
  if (!prefix.empty()) {
    cout << prefix << ",";
  }
  cout << name << "," << (success ? 1 : 0) << "," << (msg.empty() ? " " : msg);
  if (eol)
    cout << endl;
}

//----------------------------------------------------------------------------
string Timestamp() {
  time_t t;
  time(&t);
  struct tm *ts;
  ts = gmtime(&t);

  const size_t BUFSIZE = 128;
  vector<char> buf1(BUFSIZE, '\0');
  strftime(buf1.data(), BUFSIZE, "%Y%m%dT%H%M%SZ", ts);
  return string(buf1.data());
}

//------------------------------------------------------------------------------
string TempFilePath(const std::string &prefix) {
  char buf[] = "XXXXXX";
  mktemp(buf);
  string tmp(buf, buf + size(buf) / sizeof(char));
  return filesystem::temp_directory_path().string() + "/" + prefix + tmp;
}

//------------------------------------------------------------------------------
TempFile OpenTempFile(const string &mode, const std::string &prefix) {
  const string path =
      filesystem::temp_directory_path().string() + "/" + prefix + "XXXXXX";
  vector<char> buf(begin(path), end(path));
  buf.push_back('\0');
  const int fd = mkstemp(buf.data());
  const string tmpPath(begin(buf), end(buf));
  return {fdopen(fd, mode.c_str()), tmpPath};
}
