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

// Multipart upload

#include "aws_sign.h"
#include "common.h"
#include "error.h"
#include "response_parser.h"
#include "s3-api.h"
#include "s3-client.h"

using namespace std;

namespace sss {
namespace api {
namespace {

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
ETag DoUploadFilePart(S3Api &s3, const string &fileName, size_t offset,
                      size_t size, const string &bucket, const string &key,
                      const string &uploadId, int i, int tryNum,
                      S3Api::FileIOMode mode, int maxRetries, Headers headers,
                      const std::string &payloadHash) {

  try {
    headers.insert({"content-length", to_string(size)});
    const Parameters params = {{"partNumber", to_string(i + 1)},
                               {"uploadId", uploadId}};
    auto &wc = s3.Config({.method = "PUT",
                          .bucket = bucket,
                          .key = key,
                          .params = params,
                          .payloadHash = payloadHash});
    switch (mode) {
    case S3Api::BUFFERED:
      wc.UploadFile(fileName, offset, size);
      break;
    case S3Api::UNBUFFERED:
      wc.UploadFileUnbuffered(fileName, offset, size);
      break;
    case S3Api::MEMORY_MAPPED:
      wc.UploadFileMM(fileName, offset, size);
      break;
    default:
      break;
    }
    HandleError(wc);
    string etag = HTTPHeader(wc.GetHeaderText(), "Etag");
    if (etag.empty()) {
      throw(runtime_error("No ETag found in HTTP header"));
    } else {
      return TrimETag(etag);
    }
  } catch (const exception &e) {
    if (tryNum == maxRetries) {

      throw(runtime_error("Cannot upload chunk " + to_string(i + 1) + " - " +
                          e.what()));
    } else {
      retriesG++;
      return DoUploadFilePart(s3, fileName, offset, size, bucket, key, uploadId,
                              i, ++tryNum, mode, maxRetries, headers,
                              payloadHash);
    }
  }
}

//-----------------------------------------------------------------------------
ETag DoUploadPart(S3Api &s3, const string &bucket, const string &key,
                  const char *data, const string &uploadId, int i, size_t size,
                  int tryNum, int maxRetries, Headers headers,
                  const string &payloadHash) {
  const Parameters params = {{"partNumber", to_string(i + 1)},
                             {"uploadId", uploadId}};

  try {
    headers.insert({"content-length", to_string(size)});
    const auto &wc = s3.Send({.method = "PUT",
                              .bucket = bucket,
                              .key = key,
                              .params = params,
                              .uploadData = data,
                              .uploadDataSize = size});

    string etag = HTTPHeader(wc.GetHeaderText(), "Etag");
    if (etag.empty()) {
      throw(runtime_error("No ETag found in HTTP header"));
    } else {
      return TrimETag(etag);
    }

  } catch (const exception &e) {
    if (tryNum == maxRetries) {

      throw(runtime_error("Cannot upload chunk " + to_string(i + 1) + " - " +
                          e.what()));
    } else {
      retriesG++;
      return DoUploadPart(s3, bucket, key, data, uploadId, i, size, ++tryNum,
                          maxRetries, headers, payloadHash);
    }
  }
}
} // namespace
//=============================================================================
// Class implementation
//=============================================================================

ETag S3Api::CompleteMultipartUpload(const UploadId &uid, const string &bucket,
                                    const string &key,
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
    throw logic_error("Empty ETag");
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
// For Amazon Glacier:
// The part size must be a megabyte (1024 KB) multiplied by a power of 2
// for example, 1048576 (1 MB), 2097152 (2 MB), 4194304 (4 MB), 8388608 (8 MB),
// and so on. The minimum allowable part size is 1 MB, and the maximum is 4 GB.
UploadId S3Api::CreateMultipartUpload(const std::string &bucket,
                                      const std::string &key, size_t partSize,
                                      Headers headers) {
  if (partSize > 0)
    headers.insert({"x-amz-part-size", to_string(partSize)});
  const auto &wc = Send({.method = "POST",
                         .bucket = bucket,
                         .key = key,
                         .params = {{"uploads", ""}},
                         .headers = headers});
  retriesG = 0;
  const string xml = webClient_.GetContentText();
  return XMLTag(xml, "uploadId");
}

//-----------------------------------------------------------------------------
ETag S3Api::UploadPart(const std::string &bucket, const std::string &key,
                       const UploadId &uid, int partNum, const char *data,
                       size_t size, int maxRetries, Headers headers,
                       const string &payloadHash) {
  return DoUploadPart(*this, bucket, key, data, uid, partNum, size, 1,
                      maxRetries, headers, payloadHash);
}

//-----------------------------------------------------------------------------
ETag S3Api::UploadFilePart(const std::string &file, size_t offset, size_t size,
                           const std::string &bucket, const std::string &key,
                           const UploadId &uid, int partNum, FileIOMode mode,
                           int maxRetries, Headers headers,
                           const string &payloadHash) {
  return DoUploadFilePart(*this, file, offset, size, bucket, key, uid, partNum,
                          1, mode, maxRetries, headers, payloadHash);
}
//-----------------------------------------------------------------------------
void S3Api::AbortMultipartUpload(const string &bucket, const string &key,
                                 const UploadId &uid) {
  Send({.method = "DELETE",
        .bucket = bucket,
        .key = key,
        .params = {{"uploadId", uid}}});
}
} // namespace api
} // namespace sss
