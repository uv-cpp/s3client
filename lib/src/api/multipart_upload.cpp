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
//-----------------------------------------------------------------------------
void BuildUploadRequest(S3Client &s3, const string &bucket, const string &key,
                        int partNum, const string &uploadId) {
  Parameters params = {{"partNumber", to_string(partNum + 1)},
                       {"uploadId", uploadId}};
  const string &endpoint =
      s3.Endpoint(); // config.endpoints[partNum % config.endpoints.size()];
  auto signedHeaders =
      SignHeaders(s3.Access(), s3.Secret(), s3.SigningEndpoint(), "PUT", bucket,
                  key, "", params);
  Headers headers(begin(signedHeaders), end(signedHeaders));
  WebClient &wc = s3.GetWebClient();
  wc.SetHeaders(headers);
  const string path = "/" + bucket + "/" + key;
  wc.SetPath(path);
  wc.SetMethod("PUT");
  wc.SetReqParameters(params);
}

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

//-----------------------------------------------------------------------------
void BuildEndUploadRequest(S3Client &s3, const string &bucket,
                           const string &key, const vector<ETag> &etags,
                           const string &uploadId) {
  Parameters params = {{"uploadId", uploadId}};
  // const size_t ri = RandomIndex(0, int(config.endpoints.size() - 1));
  const string &endpoint = s3.Endpoint(); // config.endpoints[ri];
  auto signedHeaders = SignHeaders(s3.Access(), s3.Secret(), endpoint, "POST",
                                   bucket, key, "", params);
  Headers headers(begin(signedHeaders), end(signedHeaders));
  WebClient &wc = s3.GetWebClient();
  const string path = "/" + bucket + "/" + key;
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
ETag DoUploadPart(S3Client &s3, const string &bucket, const string &key,
                  const char *data, const string &uploadId, int i, size_t size,
                  int tryNum, int maxRetries = 1) {
  if (tryNum == 1)
    BuildUploadRequest(s3, bucket, key, i, uploadId);
  WebClient &wc = s3.GetWebClient();
  const bool ok = wc.UploadDataFromBuffer(
      data, 0, size); // UploadFile(config.file, offset, sz);
  if (!ok) {
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

ETag S3Client::CompleteMultiplartUpload(const UploadId &uid,
                                        const string &bucket, const string &key,
                                        const vector<ETag> &etags) {

  BuildEndUploadRequest(*this, bucket, key, etags, uid);
  webClient_.Send();
  if (webClient_.StatusCode() >= 400) {
    const string errcode = XMLTag(webClient_.GetContentText(), "Code");
    throw runtime_error("Error sending end upload request - " + errcode);
  }
  string etag = XMLTag(webClient_.GetContentText(), "Etag");
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
                                         const MetaDataMap &metaData) {
  webClient_.SetReqParameters({{"uploads", ""}});
  webClient_.SetMethod("POST");
  webClient_.SetPath("/" + bucket + "/" + key);
  webClient_.SetHeaders(metaData);
  webClient_.Send();
  retriesG = 0;

  if (webClient_.StatusCode() >= 400) {
    const string errcode = XMLTag(webClient_.GetContentText(), "Code");
    throw runtime_error("Error sending begin upload request - " + errcode);
  }
  const vector<uint8_t> resp = webClient_.GetResponseBody();
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
} // namespace api
} // namespace sss
