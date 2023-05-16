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
/**
 * \file s3-api.h
 * \brief declarations of \c S3Client class;
 */

#pragma once

#include "aws_sign.h"
#include "error.h"
#include "response_parser.h"
#include "s3-client.h"
#include "webclient.h"

namespace sss {

namespace api {
/**
 * \addtogroup S3_API
 * \brief S3 API
 * @{
 */
/// XML -> C++ mapping of \c ListBuckets/Buckets response.
/// https://docs.aws.amazon.com/AmazonS3/latest/API/API_ListObjectsV2.html
struct BucketInfo {
  std::string name;
  std::string creationDate; //@todo replace with std::tm
};
/// XML -> C++ mapping of \c ListObjectsV2/Contents response.
/// https://docs.aws.amazon.com/AmazonS3/latest/API/API_ListObjectsV2.html
struct ObjectInfo {
  std::string checksumAlgo;
  std::string key;
  std::string lastModified; //@todo replace with std:tm
  ETag etag;
  size_t size;
  std::string storageClass;
  // ObjectOwner
  std::string ownerDisplayName;
  std::string ownerID;
};
//@todo
// struct PartInfo {};

/**
 * \brief S3 Client inteface.
 *
 * Implements some of the S3 actions documented here:
 * https://docs.aws.amazon.com/AmazonS3/latest/API/API_Operations_Amazon_Simple_Storage_Service.html
 * Also includes higher level methods to simplify access to API.
 *
 * \section ex1 Examples
 * From test cases printing results in \c CSV format.
 *
 * \b Bucket
 * \snippet api/bucket-test.cpp CreateBucket
 * \snippet api/bucket-test.cpp ListBuckets
 * \snippet api/bucket-test.cpp DeleteBucket
 * \b Object
 * \snippet api/object-test.cpp PutObject
 * \snippet api/object-test.cpp GetObject
 * \snippet api/object-test.cpp ListObjectsV2
 * \snippet api/object-test.cpp DeleteObject
 */
class S3Api {
public:
  /// \c ListObjectsV2 request
  /// https://docs.aws.amazon.com/AmazonS3/latest/API/API_ListObjectsV2.html
  struct ListObjectV2Config {
    std::string continuationToken;
    std::string delimiter;
    std::string encodingType;
    std::string fetchOwner;
    size_t maxKeys;
    std::string prefix;
    std::string startAfter;
    // default member initializer for 'maxKeys' needed within definition of
    // enclosing class 'S3Api' outside of member functions const
    // ListObjectV2Config &config = ListObjectV2Config{}
    ListObjectV2Config() : maxKeys(0) {}
  };
  /// \c ListObjectsV2 response.
  /// https://docs.aws.amazon.com/AmazonS3/latest/API/API_ListObjectsV2.html
  /// \see ObjectInfo
  struct ListObjectV2Result {
    bool truncated;
    std::vector<ObjectInfo> keys;
  };
  /// Send request parameters.
  /// \see Send(const SendParams &p)
  /// \see Config
  struct SendParams {
    std::string method = "GET";
    std::string bucket;
    std::string key;
    Parameters params;
    Headers headers;
    std::string region = "us-east-1";
    std::string signUrl; ///< URL used for signing request headers
    std::string payloadHash;
    const std::string &postData = "";
    bool urlEncodePostParams = false; ///< if \true URL-encode \c postData else
                                      ///< send postData without encoding first
    const char *uploadData = nullptr;
    size_t uploadDataSize = 0;
  };

public:
  /// Constructor.
  ///
  /// \param[in] access access token
  ///
  /// \param[in] secret token
  ///
  /// \param[in] endpoint where reqests as sent
  ///
  /// \param[in] signingEndpoint url used to sign request, required in case
  /// requests are not sent to S3 endpoint (e.g. SSH tunnel used).
  S3Api(const std::string &access, const std::string &secret,
        const std::string &endpoint, const std::string &signingEndpoint = "")
      : access_(access), secret_(secret), endpoint_(endpoint),
        signingEndpoint_(signingEndpoint) {
    if (signingEndpoint_.empty())
      signingEndpoint_ = endpoint_;
  }
  /// No default constructor.
  S3Api() = delete;
  /// No copy constructor, \c libcurl handle cannot be copied or shared.
  S3Api(const S3Api &) = delete;
  /// Move constructor, \c libcurl handle is moved into new instance.
  S3Api(S3Api &&other)
      : access_(other.access_), secret_(other.secret_),
        endpoint_(other.endpoint_), signingEndpoint_(other.signingEndpoint_),
        webClient_(std::move(other.webClient_)) {}

public:
  /// Check if bucket exist.
  /// \param[in] bucket bucket name
  /// \return \c true if bucket exist, \c false otherwise
  bool TestBucket(const std::string &bucket);
  /// Check if key exists.
  /// \param[in] bucket bucket name
  /// \param[in] key key name
  /// \return \c true if key exists, \c false otherwise
  bool TestObject(const std::string &bucket, const std::string &key);
  /// Clear data and reset read and write functions
  void Clear() {
    webClient_.SetPath("");
    webClient_.SetHeaders({{}});
    webClient_.SetReqParameters({{}});
    webClient_.SetPostData("");
    webClient_.ClearBuffers();
    webClient_.ResetRWFunctions();
  }
  /// Send request.
  /// \param[in] p send parameters \see SendParams
  /// \return reference to \c this \c S3Api instance.
  const WebClient &Send(const SendParams &p) {
    /// [WebClient::Send]
    Config(p);
    if (ToLower(p.method) == "put") {
      if (p.uploadData)
        webClient_.UploadDataFromBuffer(p.uploadData, 0, p.uploadDataSize);
      else
        webClient_.Send();
    } else if (ToLower(p.method) == "post") {
      if (!p.postData.empty()) {
        if (p.urlEncodePostParams) {
          webClient_.SetUrlEncodedPostData(ParseParams(p.postData.data()));
        } else {
          webClient_.SetPostData(p.postData);
        }
      }
      webClient_.Send();
    } else {
      webClient_.Send();
    }
    HandleError(webClient_);
    return webClient_;
    /// [WebClient::Send]
  }

  /// Configure instance.
  /// \param[in] p configuration \see SendParams
  WebClient &Config(const SendParams &p);

  /// Send method with configurable send and receive functions.
  ///
  /// \param[in] params \see SendParams
  ///
  /// \param[in] sendFun pointer to function generating the data to send
  ///
  /// \param[in] sendUseData pointer to user data passed to \c sendFun at each
  /// invocation
  ///
  /// \param[in] receiveFun pointer to function receiving data
  ///
  /// \param[in] receiveUserData pointer to user data passed to \c receiveFun at
  /// each invocation
  void Send(const SendParams &params, WebClient::ReadFunction sendFun,
            void *sendUserData, WebClient::WriteFunction receiveFun,
            void *receiveUserData) {
    webClient_.SetWriteFunction(receiveFun, receiveUserData);
    webClient_.SetReadFunction(sendFun, sendUserData);
    Send(params);
  }

  // Higher level API
public:
  /// I/O mode used for reading and writing data from/to files.
  enum FileIOMode { BUFFERED, UNBUFFERED, MEMORY_MAPPED };
  /// Download object into file.
  /// \param[in] outFileName output file name
  /// \param[in] bucket bucket name
  /// \param[in] key key name
  /// \param[in] writeOffset write location in file
  /// \param[in] beginReadOffset offset of first byte to read from object
  /// \param[in] endReadOffset offset of last byte to read from object
  /// \param[in] headers optional headers
  void GetFileObject(const std::string &outFileName, const std::string &bucket,
                     const std::string &key, size_t writeOffset = 0,
                     size_t beginReadOffset = 0, size_t endReadOffset = 0,
                     Headers headers = {{}});

  /// Upload file to object.
  ///
  /// \param[in] infileName input file name
  ///
  /// \param[in] bucket bucket name
  ///
  /// \param[in] key key name
  ///
  /// \param[in] readOffset offset of first byte to read from file
  ///
  /// \param[in] readSize number of bytes to read from file starting at
  /// \c readOffset
  ///
  /// \return ETag of uploaded object
  ETag PutFileObject(const std::string &infileName, const std::string &bucket,
                     const std::string &key, size_t readOffset = 0,
                     size_t readSize = 0, Headers headers = {{}},
                     const std::string &payloadHash = {});

  /// Upload file part.
  /// Invoke after calling CreateMultipartUpload and before calling
  /// CompleteMultipartUpload.
  ///
  /// \param[in] inFileName input file name
  ///
  /// \param[in] readOffset offset of first byte to read in file
  ///
  /// \param[in] readSize number of bytes to read starting at \c readOffset
  ///
  /// \param[in] bucket bucket name
  ///
  /// \param[in] key key name
  ///
  /// \param[in] uid upload id returned by CreateMultipartUpload method
  ///
  /// \param[in] partNum 1-indexed part number
  ///
  /// \param[in] iomode read mode \see FileIOMode
  ///
  /// \param[in] maxRetries number of retried before aborting upload
  ///
  /// \param[in] headers optional headers
  ///
  /// \param[in] payloadHash payload hash, can be empty
  ///
  /// \return ETag of uploaded object
  ETag UploadFilePart(const std::string &inFileName, size_t readOffset,
                      size_t readSize, const std::string &bucket,
                      const std::string &key, const UploadId &uid, int partNum,
                      FileIOMode iomode = BUFFERED, int maxRetries = 1,
                      Headers headers = {{}},
                      const std::string &payloadHash = {});

  /// Return object size
  ///
  /// \param[in] bucket bucket name
  /// \param[in] key key name
  /// \return object size
  ssize_t GetObjectSize(const std::string &bucket, const std::string &key);

  // API
public:
  void AbortMultipartUpload(const std::string &bucket, const std::string &key,
                            const UploadId &);

  ETag CompleteMultipartUpload(const UploadId &uid, const std::string &bucket,
                               const std::string &key,
                               const std::vector<ETag> &etags);

  void CreateBucket(const std::string &bucket, const Headers & = {{}});

  UploadId CreateMultipartUpload(const std::string &bucket,
                                 const std::string &key, size_t partSize = 0,
                                 Headers headers = {});

  void DeleteBucket(const std::string &bucket, const Headers &headers = {{}});

  void DeleteObject(const std::string &bucket, const std::string &key,
                    const Headers & = {{}});
  // @todo
  // bool DeleteObjects(const std::string &bucket,
  //                    const std::vector<std::string> &objects);
  const CharArray &GetObject(const std::string &bucket, const std::string &key,
                             size_t begin = 0, size_t end = 0, Headers = {{}});

  /// GetObject action.
  /// \section ex1 Example
  /// \snippet api/object-test.cpp GetObject
  void GetObject(const std::string &bucket, const std::string &key,
                 CharArray &buffer, size_t offset, size_t begin = 0,
                 size_t end = 0, Headers headers = {{}});

  void GetObject(const std::string &bucket, const std::string &key,
                 char *buffer, size_t offset, size_t begin = 0, size_t end = 0,
                 Headers headers = {{}});

  Headers HeadBucket(const std::string &bucket, const Headers &headers = {{}});
  Headers HeadObject(const std::string &bucket, const std::string &key,
                     const Headers & = {{}});

  std::vector<BucketInfo> ListBuckets(const Headers &headers = {{}});

  ListObjectV2Result
  ListObjectsV2(const std::string &bucket,
                const ListObjectV2Config &config = ListObjectV2Config{},
                const Headers & = {{}});
  // @todo
  // std::vector<PartInfo> ListParts(const std::string &bucket,
  //                                 const std::string &key, const UploadId
  //                                 &uid, int max_parts);

  ETag PutObject(const std::string &bucket, const std::string &key,
                 const CharArray &buffer, Headers = {{}},
                 const std::string &payloadHash = {});

  ETag PutObject(const std::string &bucket, const std::string &key,
                 const char *, size_t size, Headers = {{}},
                 const std::string &payloadHash = {});

  ETag UploadPart(const std::string &bucket, const std::string &key,
                  const UploadId &uid, int partNum, const char *data,
                  size_t size, int maxRetries = 1, Headers headers = {{}},
                  const std::string &payloadHash = {});

public:
  /// \return access token
  const std::string &Access() const { return access_; }
  /// \return secret token
  const std::string &Secret() const { return secret_; }
  /// \return endpoint URL
  const std::string &Endpoint() const { return endpoint_; }
  /// \return signing endpoint URL
  const std::string &SigningEndpoint() const { return signingEndpoint_; }
  /// \return response body
  const std::vector<char> &GetResponseBody() const {
    return webClient_.GetResponseBody();
  }
  /// \return {header name, header value} map
  Headers GetResponseHeaders() const {
    return HTTPHeaders(webClient_.GetHeaderText());
  }

private:
  sss::WebClient webClient_;
  std::string access_;
  std::string secret_;
  std::string endpoint_;
  std::string signingEndpoint_;
};
/**
 * @}
 */
} // namespace api
} // namespace sss
