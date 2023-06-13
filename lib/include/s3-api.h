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
#include <map>
#include <variant>

namespace sss {

namespace api {
/**
 * \addtogroup S3_API
 * \brief S3 API
 * @{
 */

/// \brief XML -> C++ mapping of \c ListBuckets/Buckets response.
/// See https://docs.aws.amazon.com/AmazonS3/latest/API/API_ListObjectsV2.html
/// \code{.cpp}
/// struct BucketInfo {
///  std::string name;
///  std::string creationDate;
/// };
/// \endcode
struct BucketInfo {
  std::string name;
  std::string creationDate;
};

/// \brief XML -> C++ mapping of \c ListObjectsV2/Contents response.
/// See https://docs.aws.amazon.com/AmazonS3/latest/API/API_Object.html
/// \code{.cpp}
/// struct ObjectInfo {
///   std::string checksumAlgo;
///   std::string key;
///   std::string lastModified;
///   ETag etag;
///   size_t size;
///   std::string storageClass;
///   // ObjectOwner
///   std::string ownerDisplayName;
///   std::string ownerID;
/// };
/// \encode
struct ObjectInfo {
  std::string checksumAlgo;
  std::string key;
  std::string lastModified;
  ETag etag;
  size_t size;
  std::string storageClass;
  // ObjectOwner
  std::string ownerDisplayName;
  std::string ownerID;
};

/// \brief XML -> C++ mapping of \c Grante record
/// See https://docs.aws.amazon.com/AmazonS3/latest/API/API_GetBucketAcl.html
/// \code{.cpp}
/// struct Grant {
///   struct Grantee {
///     std::string displayName;
///     std::string emailAddress;
///     std::string id;
///     std::string xsiType;
///     std::string uri;
///     bool Empty() const {
///       return uri.empty() && xsiType.empty() && id.empty() &&
///              emailAddress.empty() && displayName.empty();
///     }
///   } grantee;
///   std::string permission;
/// };
/// \endcode
struct Grant {
  struct Grantee {
    std::string displayName;
    std::string emailAddress;
    std::string id;
    std::string xsiType;
    std::string uri;
    bool Empty() const {
      return uri.empty() && xsiType.empty() && id.empty() &&
             emailAddress.empty() && displayName.empty();
    }
  } grantee;
  std::string permission;
};

/// \brief XML -> C++ mapping of \c AccessControlPolicy
/// https://docs.aws.amazon.com/AmazonS3/latest/API/API_GetBucketAcl.html
/// \code{.cpp}
/// struct AccessControlPolicy {
///   std::string ownerDisplayName;
///   std::string ownerID;
///   std::vector<Grant> grants;
/// };
/// \endcode
struct AccessControlPolicy {
  std::string ownerDisplayName;
  std::string ownerID;
  std::vector<Grant> grants;
};

/// \brief {name, value} map representing bucket or object tags.
/// \ingroup Types
using TagMap = std::unordered_map<std::string, std::string>;
/**
 * \brief S3 Client inteface.
 *
 * Implements some of the S3 actions documented here:
 * https://docs.aws.amazon.com/AmazonS3/latest/API/API_Operations_Amazon_Simple_Storage_Service.html
 *
 * Also includes higher level methods to simplify access to the API.
 *
 * When sending requests, the fields specified as URL prameters are passed
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
 * All methods that send requests throw:
 *
 *  - \c std::runtime_error in case of errors sending the request
 *  - \c std::logic_error when the returned response code is >= 400
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
  /// \brief \c ListObjectsV2 request
  /// See https://docs.aws.amazon.com/AmazonS3/latest/API/API_ListObjectsV2.html
  /// \code{.cpp}
  /// struct ListObjectV2Config {
  ///   std::string continuationToken;
  ///   std::string delimiter;
  ///   std::string encodingType;
  ///   std::string fetchOwner;
  ///   size_t maxKeys;
  ///   std::string prefix;
  ///   std::string startAfter;
  ///   // default member initializer for 'maxKeys' needed within definition of
  ///   // enclosing class 'S3Api' outside of member functions const
  ///   // ListObjectV2Config &config = ListObjectV2Config{}
  ///   ListObjectV2Config() : maxKeys(0) {}
  /// };
  /// \endcode
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
  /// \brief \c ListObjectsV2 response.
  /// See https://docs.aws.amazon.com/AmazonS3/latest/API/API_ListObjectsV2.html
  /// \see ObjectInfo
  /// \code{.cpp}
  /// struct ListObjectV2Result {
  ///   bool truncated;
  ///   std::vector<ObjectInfo> keys;
  /// };
  /// \endcode
  struct ListObjectV2Result {
    bool truncated;
    std::vector<ObjectInfo> keys;
  };
  /// \brief Memory buffer used in SendParams
  struct ReadBuffer {
    size_t size = 0;
    const char *pData = nullptr;
  };
  /// \brief Send request parameters.
  /// \see Send(const SendParams &p)
  /// \see Config
  struct SendParams {
    std::string method = "GET"; ///< HTTP method
    std::string bucket;         ///< bucket name
    std::string key;            ///< key name
    Parameters params;          ///< url parameters as {key, value} map
    Headers headers; ///< HTTP headers as {header name, header value} map
    std::string region = "us-east-1";
    std::string signUrl;     ///< URL used for signing request headers
    std::string payloadHash; ///< payload hash, leave empty if no hash available
    bool urlEncodePostParams =
        false; ///< if \true URL-encode \c uploadData else
               ///< send \c uploadData without encoding first
    /// Data to upload through \c POST or \c PUT methods;
    /// either a \c string or a pointer to memory buffer,
    /// initialised to first type
    std::variant<std::string, ReadBuffer> uploadData;
  };

  /// \brief versioning information
  struct VersioningInfo {
    bool enabled = false;   ///< \c true if enabled
    bool mfaDelete = false; ///< \c true if MFA delete enabled
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
  /// \brief Check if bucket exist.
  /// \param[in] bucket bucket name
  /// \return \c true if bucket exist, \c false otherwise
  bool TestBucket(const std::string &bucket);
  /// \brief Check if key exists.
  /// \param[in] bucket bucket name
  /// \param[in] key key name
  /// \return \c true if key exists, \c false otherwise
  bool TestObject(const std::string &bucket, const std::string &key);
  /// \brief Clear data and reset read and write functions
  void Clear() {
    webClient_.SetPath("");
    webClient_.SetHeaders({{}});
    webClient_.SetReqParameters({{}});
    webClient_.SetPostData("");
    webClient_.ClearBuffers();
    webClient_.ResetRWFunctions();
  }
  /// \brief Send request.
  /// \param[in] p send parameters \see SendParams
  /// \return reference to \c this \c S3Api instance.
  /// \throws std::runtime_error in case of error sending request
  /// \throws std::logic_error in case of returned error (status >= 400)
  const WebClient &Send(const SendParams &p) {
    /// [WebClient::Send]
    Config(p);
    if (!HasData(p)) {
      webClient_.Send();
      HandleError(webClient_);
      return webClient_;
    }
    if (ToLower(p.method) == "put") {
      if (auto b = std::get_if<ReadBuffer>(&p.uploadData)) {
        webClient_.UploadDataFromBuffer(b->pData, 0, b->size);
      } else if (auto s = std::get_if<std::string>(&p.uploadData)) {
        webClient_.UploadDataFromBuffer(s->c_str(), 0, s->size());
      }
    } else if (ToLower(p.method) == "post") {
      if (auto s = std::get_if<std::string>(&p.uploadData)) {
        if (p.urlEncodePostParams) {
          webClient_.SetUrlEncodedPostData(ParseParams(*s));
        } else {
          webClient_.SetPostData(*s);
        }
      }
      webClient_.Send();
    }
    HandleError(webClient_);
    return webClient_;
    /// [WebClient::Send]
  }

  /// \brief Configure instance.
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
  /// \brief I/O mode used for reading and writing data from/to files.
  enum FileIOMode { BUFFERED, UNBUFFERED, MEMORY_MAPPED };
  /// \brief Download object into file.
  /// \param[in] outFileName output file name
  /// \param[in] bucket bucket name
  /// \param[in] key key name
  /// \param[in] writeOffset write location in file
  /// \param[in] beginReadOffset offset of first byte to read from object
  /// \param[in] endReadOffset offset of last byte to read from object
  /// \param[in] headers optional headers
  /// \param[in] versionId version id
  void GetFileObject(const std::string &outFileName, const std::string &bucket,
                     const std::string &key, size_t writeOffset = 0,
                     size_t beginReadOffset = 0, size_t endReadOffset = 0,
                     Headers headers = {{}}, const std::string &versionId = "");

  /// \brief Upload file to object.
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

  /// \brief Upload file part.
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

  /// \brief Return object size
  ///
  /// \param[in] bucket bucket name
  /// \param[in] key key name
  /// \param[in] versionId version id
  /// \return object size
  ssize_t GetObjectSize(const std::string &bucket, const std::string &key,
                        const std::string &versionId = "");

  // API
public:
  /// \brief Abort multipart upload
  /// \param[in] bucket bucket name
  /// \param[in] key key name
  /// \param[in] uploadId upload id returned by CreateMultipartUpload
  void AbortMultipartUpload(const std::string &bucket, const std::string &key,
                            const UploadId &uploadId);

  /// \brief Complete multipart upload
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

  /// \brief Create bucket
  ///
  /// \param[in] bucket bucket name
  ///
  /// \param[in] headers optional HTTP headers as {name, value} map
  ///
  /// \param[in] locationConstraint bucket location e.g. `ap-east-1` or `EU`
  /// refer to the updated list on the AWS web site
  ///
  /// \return bucket location
  std::string
  CreateBucket(const std::string &bucket, const Headers &headers = {{}},
               const std::string &locationConstraint = std::string());

  /// \brief Create multipart upload
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

  /// \brief Delete bucket
  ///
  /// \param[in] bucket bucket name
  ///
  /// \param[in] headers headers sent along w
  ///
  /// \param[in] headers optional HTTP headers as {name, value} map
  void DeleteBucket(const std::string &bucket, const Headers &headers = {{}});

  /// \brief Delete object
  ///
  /// \param[in] bucket bucket name
  ///
  /// \param[in] key key name
  ///
  /// \param[in] headers optional http headers as {name, value} map sent along
  /// with request
  ///
  /// \param[in] versionId version id, leave blank for latest version or in case
  /// versioning is not enabled
  void DeleteObject(const std::string &bucket, const std::string &key,
                    const Headers &headers = {},
                    const std::string &versionId = "");

  /// \brief Remove all tags from bucket
  /// \param[in] bucket bucket name
  /// \param[in] headers optional http headers as {name, value} map
  void DeleteBucketTagging(const std::string &bucket,
                           const Headers &headers = {});

  /// \brief Remove all tags from object
  /// \param[in] bucket bucket name
  /// \param[in] key name
  /// \param[in] headers optional http headers as {name, value} map
  void DeleteObjectTagging(const std::string &bucket, const std::string &key,
                           const Headers &headers = {});

  /// Return bucket's Access Control List
  /// \param bucket bucket name
  /// \return Acess Control Policy \see AccessControlPolicy
  AccessControlPolicy GetBucketAcl(const std::string &bucket);

  /// \brief Return bucket tags
  /// \param[in] bucket bucket name
  /// \return `tag name => tag value` map
  TagMap GetBucketTagging(const std::string &bucket);
  /// \brief Retrieve versioning status
  ///
  /// \param[in] bucket bucket name
  ///
  /// \return versioning info: if enabled and if MFA delete enabled
  /// \see VersioningInfo
  VersioningInfo GetBucketVersioning(const std::string &bucket);
  /// \brief Download object data
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
  /// \param[in] versionId version id, leave blank for latest version or
  /// in case versioning is not enabled
  ///
  /// \return \c char array; \c char is the type used by \c libcurl
  const CharArray &GetObject(const std::string &bucket, const std::string &key,
                             size_t beginReadOffset = 0,
                             size_t endReadOffset = 0, Headers = {{}},
                             const std::string &versionId = "");

  /// \brief Download object data into \c vector<char>
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
  ///
  /// \param[in] versionId version id, leave blank for latest version or
  /// in case versioning is not enabled
  void GetObject(const std::string &bucket, const std::string &key,
                 CharArray &outBuffer, size_t writeOffset,
                 size_t beginReadOffset = 0, size_t endReadOffset = 0,
                 Headers headers = {{}}, const std::string &versionId = "");

  /// \brief Download object data into \c char buffer
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
  ///
  /// \param[in] versionId version id, leave blank for latest version or
  /// in case versioning is not enabled
  void GetObject(const std::string &bucket, const std::string &key,
                 char *outBuffer, size_t writeOffset,
                 size_t beginReadOffset = 0, size_t endReadOffset = 0,
                 Headers headers = {{}}, const std::string &versionId = "");

  /// \brief Return bucket's Access Control List
  /// \param bucket bucket name
  /// \param key key name
  /// \return Acess Control Policy \see AccessControlPolicy
  AccessControlPolicy GetObjectAcl(const std::string &bucket,
                                   const std::string &key);
  /// \brief Return object tags
  /// \param[in] bucket bucket name
  /// \param[in] key key name
  /// \return `tag name => tag value` map
  TagMap GetObjectTagging(const std::string &bucket, const std::string &key);

  /// Send \c HeadBucket request
  ///
  /// \param[in] bucket bucket name
  ///
  /// \param[in] optional http headers as {name, value} map sent along with
  /// request
  ///
  /// \return HTTP headers as {http header name, value} map
  Headers HeadBucket(const std::string &bucket, const Headers &headers = {{}});

  /// \brief Send \c HeadObject request
  ///
  /// \param[in] bucket bucket name
  ///
  /// \param[in] key key name
  ///
  /// \param[in] optional http headers as {name, value} map sent along with
  /// request
  ///
  /// \param[in] versionId version id, leave blank for latest version or in
  /// case versioning is not enabled
  ///
  /// \return HTTP headers as {http header name, value} map
  Headers HeadObject(const std::string &bucket, const std::string &key,
                     const Headers & = {{}}, const std::string &versionId = "");

  /// \brief List buckets
  ///
  /// \param[in] headers optional http headers as {name, value} map sent along
  /// with request
  ///
  /// \return bucket list \see BucketInfo
  std::vector<BucketInfo> ListBuckets(const Headers &headers = {{}});

  /// \brief List objects by sending a \c ListObjectsV2 request
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

  /// \brief Set Access Control Policy for bucket.
  ///
  /// \param[in] bucket bucket name
  ///
  /// \param[in] acl Acess Control Policy \see AccessControlPolicy
  void PutBucketAcl(const std::string &bucket, const AccessControlPolicy &acl);

  /// \brief Tag bucket
  /// \param[in] bucket bucket name
  /// \param[in] tags tag map: {tag name => tag value}
  /// \param[in] http headers as {header name, header value} map
  void PutBucketTagging(const std::string &bucket, const TagMap &tags,
                        const Headers &headers = {});
  /// \brief Enable bucket versioning.
  /// \param[in] bucket bucket name
  /// \param[in] enables \c true to enable, \c false to disable
  void PutBucketVersioning(const std::string &bucket, bool enabled);
  /// \brief Upload data to object by sending a \c PutObject request
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

  /// \brief Upload data to object by sending a \c PutObject request
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

  /// \brief Set Access Control Policy
  /// \param bucket[in] bucket name
  /// \param key[in] key name
  /// \param acl[in] access control policy \see AccessControlPolicy
  void PutObjectAcl(const std::string &bucket, const std::string &key,
                    const AccessControlPolicy &acl);

  /// \brief Set object tags
  /// \param[in] bucket bucket name
  /// \param[in] key key name
  /// \param[in] tags tag map
  /// \param[in] headers http headers as {header name, header value} map
  void PutObjectTagging(const std::string &bucket, const std::string &key,
                        const TagMap &tags, const Headers &headers = {});

  /// \brief Upload part
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
  /// \return response body as text
  std::string GetResponseText() const { return webClient_.GetContentText(); }

  /// \brief Get returned HTTP headers.
  /// Use this method to retrieve additional information e.g. \c versionId
  /// after sending a request
  /// \return {header name, header value} map
  Headers GetResponseHeaders() const {
    return HTTPHeaders(webClient_.GetHeaderText());
  }

private:
  bool HasData(const SendParams &params) {
    bool hasData = false;
    if (auto p = std::get_if<ReadBuffer>(&params.uploadData)) {
      hasData = p->size > 0 && p->pData != nullptr;
    } else if (auto p = std::get_if<std::string>(&params.uploadData)) {
      hasData = !p->empty();
    }
    return hasData;
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
