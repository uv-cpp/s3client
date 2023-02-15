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

//-----------------------------------------------------------------------------
WebClient BuildUploadRequest(const S3FileTransferConfig &config,
                             const string &path, int partNum,
                             const string &uploadId) {
  Parameters params = {{"partNumber", to_string(partNum + 1)},
                       {"uploadId", uploadId}};
  const string endpoint = config.endpoints[partNum % config.endpoints.size()];
  auto signedHeaders =
      SignHeaders(config.accessKey, config.secretKey, endpoint, "PUT",
                  config.bucket, config.key, "", params);
  Headers headers(begin(signedHeaders), end(signedHeaders));
  WebClient req(endpoint, path, "PUT", params, headers);
  return req;
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
WebClient BuildEndUploadRequest(const S3FileTransferConfig &config,
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
  WebClient req(endpoint, path, "POST", params, headers);
  req.SetPostData(BuildEndUploadXML(etags));
  return req;
}

namespace {
atomic<int> retriesG;
}

//-----------------------------------------------------------------------------
int GetUploadRetries() { return retriesG; }

//-----------------------------------------------------------------------------
string UploadPart(const S3FileTransferConfig &config, const string &path,
                  const string &uploadId, int i, size_t chunkSize,
                  size_t fileSize, int tryNum) {
  WebClient ul = BuildUploadRequest(config, path, i, uploadId);
  const size_t sz = min(chunkSize, fileSize - chunkSize * i);
  const size_t offset = chunkSize * i;
  const bool ok = ul.UploadFile(config.file, offset, sz);
  if (!ok) {
    if (tryNum == config.maxRetries) {
      throw(runtime_error("Cannot upload chunk " + to_string(i + 1)));
    } else {
      retriesG++;
      return UploadPart(config, path, uploadId, i, offset, chunkSize, ++tryNum);
    }
  } else {
    string etag = HTTPHeader(ul.GetHeaderText(), "Etag");
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

//-----------------------------------------------------------------------------
vector<string> UploadParts(const S3FileTransferConfig &args, const string &path,
                           const string &uploadId, size_t chunkSize,
                           int firstPart, int lastPart, size_t fileSize) {
  vector<string> etags;
  for (int i = firstPart; i != lastPart; ++i) {
    etags.push_back(
        UploadPart(args, path, uploadId, i, chunkSize, fileSize, 1));
  }
  return etags;
}
/*ETag*/ ::std::string UploadFile(const S3FileTransferConfig &config,
                                  const MetaDataMap &metaData) {
  retriesG = 0;
  FILE *inputFile = fopen(config.file.c_str(), "rb");
  if (!inputFile) {
    throw runtime_error(string("cannot open file ") + config.file);
  }
  fclose(inputFile);
  // retrieve file size
  const size_t fileSize = sss::FileSize(config.file);
  string path = "/" + config.bucket + "/" + config.key;
  if (config.endpoints.empty())
    throw std::logic_error("Missing endpoint information");
  const string endpoint =
      config.endpoints[RandomIndex(0, config.endpoints.size() - 1)];
  if (config.jobs > 1) {
    S3ClientConfig args;
    // begin uplaod request -> get upload id
    args.accessKey = config.accessKey;
    args.secretKey = config.secretKey;
    args.endpoint = config.endpoints.front();
    args.bucket = config.bucket;
    args.key = config.key;
    args.method = "POST";
    args.params = {{"uploads", ""}};
    args.headers = metaData;
    auto req = SendS3Request(args);
    // initiate request
    if (req.StatusCode() >= 400) {
      const string errcode = XMLTag(req.GetContentText(), "Code");
      throw runtime_error("Error sending begin upload request - " + errcode);
    }
    vector<uint8_t> resp = req.GetResponseBody();
    const string xml(begin(resp), end(resp));
    const string uploadId = XMLTag(xml, "uploadId");
    const size_t numParts = config.chunksPerJob * config.jobs;
    const size_t partsPerJob = (numParts + config.jobs - 1) / config.jobs;
    const size_t chunkSize = (fileSize + numParts - 1) / numParts;
    // send parts in parallel and store ETags
    vector<future<vector<string>>> etags(config.jobs);
    for (int i = 0; i != config.jobs; ++i) {
      const size_t parts = min(partsPerJob, numParts - partsPerJob * i);
      etags[i] =
          async(launch::async, UploadParts, config, path, uploadId, chunkSize,
                i * partsPerJob, i * partsPerJob + parts, fileSize);
    }
    // send end upload request
    WebClient endUpload = BuildEndUploadRequest(config, path, etags, uploadId);
    endUpload.Send();
    if (endUpload.StatusCode() >= 400) {
      const string errcode = XMLTag(endUpload.GetContentText(), "Code");
      throw runtime_error("Error sending end upload request - " + errcode);
    }
    string etag = XMLTag(endUpload.GetContentText(), "Etag");
    string etagX = HTTPHeader(req.GetHeaderText(), "Etag");
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
  } else {
    auto signedHeaders =
        SignHeaders(config.accessKey, config.secretKey, endpoint, "PUT",
                    config.bucket, config.key, "", {}, metaData);
    Map headers(begin(signedHeaders), end(signedHeaders));
    WebClient req(endpoint, path, "PUT", {}, headers);
    if (!req.UploadFile(config.file)) {
      throw runtime_error("Error sending request: " + req.ErrorMsg());
    }
    if (req.StatusCode() >= 400) {
      const string errcode = XMLTag(req.GetContentText(), "Code");
      throw runtime_error("Error sending upload request - " + errcode);
    }

    string etag = HTTPHeader(req.GetHeaderText(), "Etag");
    if (etag[0] == '"') {
      etag = etag.substr(1, etag.size() - 2);
    }
    if (etag.empty()) {
      throw runtime_error("Error sending upload request");
    }
    return etag;
  }
}
} // namespace sss
