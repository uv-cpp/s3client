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
#include "error.h"
#include "response_parser.h"
#include "s3-api.h"
#include "s3-client.h"

#include <fstream>
#include <future>
#include <set>
#include <thread>
using namespace std;

namespace sss {
namespace api {
namespace {
// //-----------------------------------------------------------------------------
// void BuildUploadRequest(S3Client &s3, const string &bucket, const string
// &key,
//                         int partNum, const string &uploadId) {
//   Parameters params = {{"partNumber", to_string(partNum + 1)},
//                        {"uploadId", uploadId}};
//   const string &endpoint =
//       s3.Endpoint(); // config.endpoints[partNum % config.endpoints.size()];
//   auto signedHeaders =
//       SignHeaders(s3.Access(), s3.Secret(), s3.SigningEndpoint(), "PUT",
//       bucket,
//                   key, "", params);
//   Headers headers(begin(signedHeaders), end(signedHeaders));
//   s3.Clear();
//   WebClient &wc = s3.GetWebClient();
//   wc.SetHeaders(headers);
//   const string path = "/" + bucket + "/" + key;
//   wc.SetPath(path);
//   wc.SetMethod("PUT");
//   wc.SetReqParameters(params);
// }

//-----------------------------------------------------------------------------
string BuildEndUploadXML(const vector<ETag> &etags) {
  string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
               "<CompleteMultipartUpload "
               "xmlns=\"http://s3.amazonaws.com/doc/2006-03-01/\">\n";
  int globalIndex = 0;
  for (const auto &es : etags) {
    const string part = "<Part><ETag>" + es + "</ETag><PartNumber>" +
                        to_string(globalIndex + 1) + "</PartNumber></Part>";
    xml += part;
    ++globalIndex;
  }
  xml += "</CompleteMultipartUpload>";
  return xml;
}

atomic<int> retriesG;

//-----------------------------------------------------------------------------
int GetUploadRetries() { return retriesG; }

//-----------------------------------------------------------------------------
ETag DoUploadPart(S3Client &s3, const string &bucket, const string &key,
                  const char *data, const string &uploadId, int i, size_t size,
                  int tryNum, int maxRetries = 1) {
  const Parameters params = {{"partNumber", to_string(i + 1)},
                             {"uploadId", uploadId}};
  const auto &wc = s3.Send({.method = "PUT",
                            .bucket = bucket,
                            .key = key,
                            .params = params,
                            .uploadData = data,
                            .uploadDataSize = size});

  if (false) {
    if (tryNum == maxRetries) {

      throw(runtime_error("Cannot upload chunk " + to_string(i + 1)));
    } else {
      retriesG++;
      return DoUploadPart(s3, bucket, key, data, uploadId, i, size, ++tryNum,
                          maxRetries);
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
} // namespace
//=============================================================================
// Class implementation
//=============================================================================

ETag S3Client::CompleteMultipartUpload(const UploadId &uid,
                                       const string &bucket, const string &key,
                                       const vector<ETag> &etags) {

  Parameters params = {{"uploadId", uid}};
  const string postData = BuildEndUploadXML(etags);
  const auto &wc = Send({.method = "POST",
                         .bucket = bucket,
                         .key = key,
                         .params = params,
                         .postData = postData});
  string etag = XMLTag(wc.GetContentText(), "ETag");
  if (etag.empty()) {
    throw runtime_error("Empty ETag");
  }
  if (etag[0] == '"') {
    etag = etag.substr(1, etag.size() - 2);
  } else if (etag.substr(0, string("&#34;").size()) == "&#34;") {
    const size_t quotes = string("&#34;").size();
    etag = etag.substr(quotes, etag.size() - 2 * quotes);
  }
  return etag;
}

//-----------------------------------------------------------------------------
UploadId S3Client::CreateMultipartUpload(const std::string &bucket,
                                         const std::string &key,
                                         const Headers &headers) {
  const auto &wc = Send({.method = "POST",
                         .bucket = bucket,
                         .key = key,
                         .params = {{"uploads", ""}},
                         .headers = headers});
  retriesG = 0;
  const vector<char> &resp = webClient_.GetResponseBody();
  const string xml(begin(resp), end(resp));
  return XMLTag(xml, "uploadId");
}

//-----------------------------------------------------------------------------
ETag S3Client::UploadPart(const std::string &bucket, const std::string &key,
                          const UploadId &uid, int partNum, const char *data,
                          size_t size, int maxRetries) {
  return DoUploadPart(*this, bucket, key, data, uid, partNum, size, 1,
                      maxRetries);
}

//-----------------------------------------------------------------------------
void S3Client::AbortMultipartUpload(const string &bucket, const string &key,
                                    const UploadId &uid) {
  Clear();
  webClient_.SetMethod("DELETE");
  webClient_.SetPath(bucket + "/" + key);
  webClient_.SetReqParameters({{"uploadId", uid}});
  webClient_.Send();
  HandleError(webClient_);
}

} // namespace api
} // namespace sss
