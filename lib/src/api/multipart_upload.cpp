
/*******************************************************************************
 * BSD 3-Clause License
 *
 * Copyright (c) 2020-2022, Ugo Varetto
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

// Upload files

#include "aws_sign.h"
#include "common.h"
#include "response_parser.h"
#include "s3-client.h"

#include <fstream>
#include <future>
#include <set>
#include <thread>
using namespace std;

namespace sss {
namespace api {
namespace {
//-----------------------------------------------------------------------------
void BuildUploadRequest(WebClient &wc, const S3FileTransferConfig &config,
                        const string &path, int partNum,
                        const string &uploadId) {
  Parameters params = {{"partNumber", to_string(partNum + 1)},
                       {"uploadId", uploadId}};
  const string endpoint = config.endpoints[partNum % config.endpoints.size()];
  auto signedHeaders =
      SignHeaders(config.accessKey, config.secretKey, endpoint, "PUT",
                  config.bucket, config.key, "", params);
  Headers headers(begin(signedHeaders), end(signedHeaders));
  wc.SetHeaders(headers);
  wc.SetPath(path);
  wc.SetMethod("PUT");
  wc.SetReqParameters(params);
}

//-----------------------------------------------------------------------------
string BuildEndUploadXML(vector<future<vector<string>>> &etags) {
  string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
               "<CompleteMultipartUpload "
               "xmlns=\"http://s3.amazonaws.com/doc/2006-03-01/\">\n";
  int globalIndex = 0;
  for (auto &ess : etags) {
    if (!ess.valid()) {
      throw runtime_error("Error - request " + to_string(globalIndex));
      return "";
    }
    for (const auto &es : ess.get()) {
      const string part = "<Part><ETag>" + es + "</ETag><PartNumber>" +
                          to_string(globalIndex + 1) + "</PartNumber></Part>";
      xml += part;
      ++globalIndex;
    }
  }
  xml += "</CompleteMultipartUpload>";
  return xml;
}

//-----------------------------------------------------------------------------
void BuildEndUploadRequest(WebClient &wc, const S3FileTransferConfig &config,
                           const string &path,
                           vector<future<vector<string>>> &etags,
                           const string &uploadId) {
  Parameters params = {{"uploadId", uploadId}};
  const size_t ri = RandomIndex(0, int(config.endpoints.size() - 1));
  const string endpoint = config.endpoints[ri];
  auto signedHeaders =
      SignHeaders(config.accessKey, config.secretKey, endpoint, "POST",
                  config.bucket, config.key, "", params);
  Headers headers(begin(signedHeaders), end(signedHeaders));
  wc.SetMethod("POST");
  wc.SetReqParameters(params);
  wc.SetPath(path);
  wc.SetHeaders(signedHeaders);
  wc.SetPostData(BuildEndUploadXML(etags));
}

atomic<int> retriesG;

//-----------------------------------------------------------------------------
int GetUploadRetries() { return retriesG; }

//-----------------------------------------------------------------------------
string UploadPart(WebClient &wc, const char *data, const string &path,
                  const string &uploadId, int i, size_t chunkSize, size_t size,
                  int tryNum) {
  if (tryNum == 1)
    BuildUploadRequest(wc, path, i, uploadId);
  const size_t sz = min(chunkSize, size - chunkSize * i);
  const size_t offset = chunkSize * i;
  const bool ok =
      wc.UploadData(data, offset, sz); // UploadFile(config.file, offset, sz);
  if (!ok) {
    if (tryNum == config.maxRetries) {
      throw(runtime_error("Cannot upload chunk " + to_string(i + 1)));
    } else {
      retriesG++;
      return UploadPart(wc, config, path, uploadId, i, offset, chunkSize,
                        ++tryNum);
    }
  } else {
    string etag = HTTPHeader(wc.GetHeaderText(), "Etag");
    if (etag.empty()) {
      throw(runtime_error("No ETag found in HTTP header"));
    } else {
      if (etag[0] == '"') {
        etag = etag.substr(1, etag.size() - 2);
      } else if (etag.substr(0, string("&#34;").size()) == "&#34;") {
        const size_t quotes = string("&#34;").size();
        etag = etag.substr(quotes, etag.size() - 2 * quotes);
      }
      return etag;
    }
  }
}
}
} // namespace api
} // namespace sss
