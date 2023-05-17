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
 * \brief declarations of \c S3Api class.
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

/// XML -> C++ mapping of \c Gratee
/// https://docs.aws.amazon.com/AmazonS3/latest/API/API_GetBucketAcl.html
struct Grantee {
  std::string displayName;
  std::string emailAddress;
  std::string id;
  std::string xsiType;
  std::string uri;
};

/// XML -> C++ mapping of \c AccessControlPolicy
/// https://docs.aws.amazon.com/AmazonS3/latest/API/API_GetBucketAcl.html
struct AccessControlPolicy {
  std::string ownerDisplayName;
  std::string ownerID;
  std::vector<Grantee> grants;
  std::string permission;
};
//@todo
// struct PartInfo {};

/**
 * \brief S3 Client inteface.
 *
 * Implements some of the S3 actions documented here:
 * https://docs.aws.amazon.com/AmazonS3/latest/API/API_Operations_Amazon_Simple_Storage_Service.html
 *
 * Also includes higher level methods to simplify access to the API.
 *
 * When sending requests, the fields specified as URL prameters are are passed
 * to methods as C++ structs and sent as part of the request, but in most cases
 * the optional parameters sent as HTTP header fields are not sent along, it is
 * therefore required to explicitly pass them in the \c headers parameter.
 *
 * E.g. the \c ListObjectV2 request supports HTTP
 * headers for specifying the payer for the request and owner of the listed
 * objects. \code{.cpp} s3.ListObjectsV2(bucket, config, {
 *     {"x-amz-request-payer", "RequestPayer"},
 *     {"x-amz-expected-bucket-owner", "ExpectedBucketOwner"}
 * });
 * \endcode
 *
 * <h2>Error handling</h2>
 *
 * Currently exceptions are being used to report errors, work is ongoing to
 * move to result types similar to Rust's \c Result<ResultT,ErrorT>.
 *
 * All methods that send request throw:
 *
 *  - \c std::runtime_error in case of errors sending the request
 *  - \c std::logic_error when the returned response
 *
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
  /// \throws std::runtime_error in case of error sending request
  /// \throws std::logic_error in case of returned error (status >= 400)
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

  // High level API
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
  /// \param[in] partNum zero-indexed part number
  ///
  /// \param[in] iomode read mode \see FileIOMode
  ///
  /// \param[in] maxRetries number of retried before aborting upload
  ///
  /// \param[in] headers optional HTTP headers as {name, value} map
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
  /// Abort multipart upload
  /// \param[in] bucket bucket name
  /// \param[in] key key name
  /// \param[in] uploadId upload id returned by CreateMultipartUpload
  void AbortMultipartUpload(const std::string &bucket, const std::string &key,
                            const UploadId &uploadId);

  /// Complete multipart upload
  /// \param[in] uid upload id returned by CreateMultipartUpload
  /// \param[in] bucket bucket name
  /// \param[in] key key name
  /// \param[in] etags etags of uploaded parts
  /// \return etag of multipart upload
  /// \throws std::runtime_error in case of error sending request
  /// \throws std::logic_error in case of returned error (status >= 400)
  ETag CompleteMultipartUpload(const UploadId &uid, const std::string &bucket,
                               const std::string &key,
                               const std::vector<ETag> &etags);

  /// Create bucket
  ///
  /// \param[in] bucket bucket name
  ///
  /// \param[in] headers optional HTTP headers as {name, value} map
  void CreateBucket(const std::string &bucket, const Headers &headers = {{}});

  /// Create multipart upload
  ///
  /// \param[in] bucket bucket name
  ///
  /// \param[in] key key name
  ///
  /// \param[in] partSize part size hint, optional
  ///
  /// \param[in] headers optional HTTP headers as {name, value} map
  UploadId CreateMultipartUpload(const std::string &bucket,
                                 const std::string &key, size_t partSize = 0,
                                 Headers headers = {});

  /// Delete bucket
  ///
  /// \param[in] bucket bucket name
  ///
  /// \param[in] headers headers sent along w
  ///
  /// \param[in] headers optional HTTP headers as {name, value} map
  void DeleteBucket(const std::string &bucket, const Headers &headers = {{}});

  /// Delete object
  ///
  /// \param[in] bucket bucket name
  ///
  /// \param[in] key key name
  ///
  /// \param[in] optional http headers as {name, value} map sent along with
  /// request
  void DeleteObject(const std::string &bucket, const std::string &key,
                    const Headers & = {{}});
  // @todo
  // bool DeleteObjects(const std::string &bucket,
  //                    const std::vector<std::string> &objects);

  /// Return bucket's Access Control List
  /// \param bucket bucket name
  /// \return Acess Control Policy \see AccessControlPolicy
  AccessControlPolicy GetBucketAcl(const std::string &bucket);

  /// Download object data
  ///
  /// \param[in] bucket bucket name
  ///
  /// \param[in] key key name
  ///
  /// \param[in] beginReadOffset offset of first byte read from object
  ///
  /// \param[in] endReadOffset offset of last byte read from object
  ///
  /// \param[in] optional http headers as {name, value} map sent along with
  /// request
  ///
  /// \return \c char array; \c char is the type used by \c libcurl
  const CharArray &GetObject(const std::string &bucket, const std::string &key,
                             size_t beginReadOffset = 0,
                             size_t endReadOffset = 0, Headers = {{}});

  /// Download object data into \c vector<char>
  ///
  /// \param[in] bucket bucket name
  ///
  /// \param[in] key key name
  ///
  /// \param[out] outBuffer output buffer
  ///
  /// \param[in] writeOffset offset in output buffer
  ///
  /// \param[in] beginReadOffset offset of first byte read from object
  ///
  /// \param[in] endReadOffset offset of last byte read from object
  ///
  /// \param[in] optional http headers as {name, value} map sent along with
  /// request
  void GetObject(const std::string &bucket, const std::string &key,
                 CharArray &outBuffer, size_t writeOffset,
                 size_t beginReadOffset = 0, size_t endReadOffset = 0,
                 Headers headers = {{}});

  /// Download object data into \c char buffer
  ///
  /// \param[in] bucket bucket name
  ///
  /// \param[in] key key name
  ///
  /// \param[out] outBuffer output buffer
  ///
  /// \param[in] writeOffset offset in output buffer
  ///
  /// \param[in] beginReadOffset offset of first byte read from object
  ///
  /// \param[in] endReadOffset offset of last byte read from object
  ///
  /// \param[in] optional http headers as {name, value} map sent along with
  /// request
  void GetObject(const std::string &bucket, const std::string &key,
                 char *outBuffer, size_t writeOffset,
                 size_t beginReadOffset = 0, size_t endReadOffset = 0,
                 Headers headers = {{}});

  /// Send \c HeadBucket request
  ///
  /// \param[in] bucket bucket name
  ///
  /// \param[in] optional http headers as {name, value} map sent along with
  /// request
  ///
  /// \return HTTP headers as {http header name, value} map
  Headers HeadBucket(const std::string &bucket, const Headers &headers = {{}});

  /// Send \c HeadObject request
  ///
  /// \param[in] bucket bucket name
  ///
  /// \param[in] key key name
  ///
  /// \param[in] optional http headers as {name, value} map sent along with
  /// request
  ///
  /// \return HTTP headers as {http header name, value} map
  Headers HeadObject(const std::string &bucket, const std::string &key,
                     const Headers & = {{}});

  /// List buckets
  ///
  /// \param[in] headers optional http headers as {name, value} map sent along
  /// with request
  ///
  /// \return bucket list \see BucketInfo
  std::vector<BucketInfo> ListBuckets(const Headers &headers = {{}});

  /// List objects by sending a \c ListObjectsV2 request
  ///
  /// \param[in] bucket bucket name
  ///
  /// \param[in] config optional configuration parameters \see
  /// ListObjectV2Config
  ///
  /// \param[in] headers optional http headers as {name, value} map sent along
  /// with request
  ///
  /// \return object list \see ListObjectV2Result
  ListObjectV2Result
  ListObjectsV2(const std::string &bucket,
                const ListObjectV2Config &config = ListObjectV2Config{},
                const Headers & = {{}});
  // @todo
  // std::vector<PartInfo> ListParts(const std::string &bucket,
  //                                 const std::string &key, const UploadId
  //                                 &uid, int max_parts);

  /// Upload data to object by sending a \c PutObject request
  /// \param[in] bucket bucket name
  ///
  /// \param[in] key key name
  ///
  /// \param[in] buffer input buffer
  ///
  /// \param[in] headers optional http headers as {header name, value} map
  ///
  /// \param[in] optional payloadHash payload hash can be empty
  ///
  /// \return etag
  ETag PutObject(const std::string &bucket, const std::string &key,
                 const CharArray &buffer, Headers headers = {{}},
                 const std::string &payloadHash = {});

  /// Upload data to object by sending a \c PutObject request
  /// \param[in] bucket bucket name
  ///
  /// \param[in] key key name
  ///
  /// \param[in] buffer input buffer
  ///
  /// \param[in] headers optional HTTP headers as {header name, value} map
  ///
  /// \param[in] optional payloadHash payload hash can be empty
  ///
  /// \return etag
  ETag PutObject(const std::string &bucket, const std::string &key,
                 const char *buffer, size_t size, Headers headers = {{}},
                 const std::string &payloadHash = {});

  /// Upload part
  ///
  /// \param[in] bucket bucket name
  ///
  /// \param[in] key key name
  ///
  /// \param[in] uid upload id returned by CreateMultipartUpload
  ///
  /// \param[in] partNum zero-indexed part number
  ///
  /// \param[in] data input buffer
  ///
  /// \param[in] size data size
  ///
  /// \param[in] maxRetries maximum number of retries before aborting upload
  ///
  /// \param[in] headers optional HTTP headers as {header name, value} map
  ///
  /// \param[in] payloadHash optional payload hash, can be empty
  ///
  /// \return etag
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
