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

// Send S3v4 signed REST requests

#include "s3-client.h"
#include "aws_sign.h"
#include "common.h"

#include <fstream>
#include <future>
#include <set>
#include <thread>

using namespace std;
using namespace sss;

//------------------------------------------------------------------------------
void Validate(const S3Args &args) {
  if (args.s3AccessKey.empty() && !args.s3SecretKey.empty() ||
      args.s3SecretKey.empty() && !args.s3AccessKey.empty()) {
    throw invalid_argument(
        "ERROR: both access and secret keys have to be specified");
  }
#ifdef VALIDATE_URL
  const URL url = ParseURL(args.endpoint);
  if (url.proto != "http" && url.proto != "https") {
    throw invalid_argument(
        "ERROR: only 'http' and 'https' protocols supported");
  }
  regex re(R"((\w+\.)*\w+\.\w+)");
  if (!regex_match(url.host, re)) {
    throw invalid_argument("ERROR: invalid endpoint format, should be "
                           "http[s]://hostname[:port]");
  }
  if (url.port > 0xFFFF) {
    throw invalid_argument(
        "ERROR: invalid port number, should be in range[1-65535]");
  }
#endif
  const set<string> methods({"get", "put", "post", "delete", "head"});
  if (methods.count(ToLower(args.method)) == 0) {
    throw invalid_argument(
        "ERROR: only 'get', 'put', 'post', 'delete', 'head' methods "
        "supported");
  }
}

//-----------------------------------------------------------------------------
WebClient SendS3Request(S3Args &args) {
  // verify peer and host certificate only if signing url == endpoint
  const bool verify = args.signUrl.empty();
  ///\warning disabling verification for both peer and host
  const bool verifyHost = verify;
  const bool verifyPeer = verify;
  if (args.signUrl.empty())
    args.signUrl = args.endpoint;
  string path;
  if (!args.bucket.empty()) {
    path += "/" + args.bucket;
    if (!args.key.empty())
      path += "/" + args.key;
  }
  const Map params = ParseParams(args.params);
  Map headers = ParseHeaders(args.headers);
  if (!args.s3AccessKey.empty()) {
    auto signedHeaders =
        SignHeaders(args.s3AccessKey, args.s3SecretKey, args.signUrl,
                    args.method, args.bucket, args.key, "", params, headers);
    headers.insert(begin(signedHeaders), end(signedHeaders));
  }
  WebClient req(args.endpoint, path, args.method, params, headers);
  req.SSLVerify(verifyPeer, verifyHost);
  FILE *of = NULL;
  if (!args.outfile.empty()) {
    of = fopen(args.outfile.c_str(), "wb");
    req.SetWriteFunction(NULL, of); // default is to write to file
  }
  if (!args.data.empty()) {
    if (args.data[0] != '@') {
      if (ToLower(args.method) == "post") {
        req.SetUrlEncodedPostData(ParseParams(args.data));
        req.SetMethod("POST");
        req.Send();
      } else { // "put"
        vector<uint8_t> data(begin(args.data), end(args.data));
        // req.SetUploadData(data);
        // req.Send();
        req.UploadDataFromBuffer(args.data.c_str(), 0, data.size());
      }
    } else {
      if (ToLower(args.method) == "put") {
        req.UploadFile(args.data.substr(1));
      } else if (args.method == "post") {
        ifstream t(args.data.substr(1));
        const string str((istreambuf_iterator<char>(t)),
                         istreambuf_iterator<char>());
        req.SetMethod("POST");
        req.SetPostData(str);
        req.Send();
      } else {
        throw domain_error("Wrong method " + args.method);
      }
    }
  } else
    req.Send();
  if (of)
    fclose(of);
  return req;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
struct DArgs {
  string s3AccessKey;
  string s3SecretKey;
  string endpoint;
  string bucket;
  string key;
  string file;
  int jobs = 1;
};
namespace sss {
extern string HTTPHeader(const string &, const string &);
}
size_t ObjectSize(const string &s3AccessKey, const string &s3SecretKey,
                  const string &endpoint, const string &bucket,
                  const string &key, const string &signUrl = "") {
  S3Args args;
  args.s3AccessKey = s3AccessKey;
  args.s3SecretKey = s3SecretKey;
  args.endpoint = endpoint;
  args.signUrl = signUrl;
  args.bucket = bucket;
  args.key = key;
  auto req = SendS3Request(args);
  const vector<uint8_t> h = req.GetResponseHeader();
  const string hs(begin(h), end(h));
  const string cl = sss::HTTPHeader(hs, "[Cc]ontent-[Ll]ength");
  char *ns;
  return size_t(strtoull(cl.c_str(), &ns, 10));
}
// Extract bytes from object by specifying range in HTTP header:
// E.g. "Range: bytes=100-1000"
int DownloadPart(const DArgs &args, const string &path, int id,
                 size_t chunkSize, size_t lastChunkSize) {
  auto signedHeaders = SignHeaders(args.s3AccessKey, args.s3SecretKey,
                                   args.endpoint, "GET", args.bucket, args.key);
  Headers headers(begin(signedHeaders), end(signedHeaders));
  const size_t sz = id < args.jobs - 1 ? chunkSize : lastChunkSize;
  const string range = "bytes=" + to_string(id * chunkSize) + "-" +
                       to_string(id * chunkSize + sz - 1);
  headers.insert({"Range", range});
  WebClient req(args.endpoint, path, "GET", {}, headers);
  FILE *out = fopen(args.file.c_str(), "wb");
  fseek(out, id * chunkSize, SEEK_SET);
  req.SetWriteFunction(NULL, out);
  req.Send();
  return req.StatusCode();
}

void DownloadFile(const std::string &s3AccessKey,
                  const std::string &s3SecretKey, const std::string &endpoint,
                  const std::string &bucket, const std::string &key,
                  const std::string &file, int jobs, int retries,
                  const std::string &signUrl) {

  vector<future<int>> status(jobs);
  // retrieve file size from remote object
  const size_t fileSize =
      ObjectSize(s3AccessKey, s3SecretKey, endpoint, bucket, key, signUrl);
  // compute chunk size
  const size_t chunkSize = (fileSize + jobs - 1) / jobs;
  // compute last chunk size
  const size_t lastChunkSize = fileSize - chunkSize * (jobs - 1);
  // create output file
  std::ofstream ofs(file, std::ios::binary | std::ios::out);
  ofs.seekp(fileSize);
  ofs.write("", 1);
  ofs.close();
  // initiate request
  string path = "/" + bucket + "/" + key;
  DArgs args = {s3AccessKey, s3SecretKey, endpoint, bucket, key, file, jobs};
  for (size_t i = 0; i != jobs; ++i) {
    status[i] = async(launch::async, DownloadPart, args, path, i, chunkSize,
                      lastChunkSize);
  }
  for (auto &i : status) {
    if (i.get() > 300)
      throw runtime_error("Error downloading file");
  }
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//------------------------------------------------------------------------------
struct UploadConfig {
  string s3AccessKey;
  string s3SecretKey;
  string endpoint;
  string bucket;
  string key;
  string file;
  string credentials;
  string awsProfile;
  vector<string> endpoints;
  int maxRetries = 2;
  int jobs = 1;
};

vector<string> ReadEndpoints(const string &fname) {
  vector<string> ep;
  ifstream in(fname);
  if (in.fail()) {
    throw runtime_error("Cannot open configuration file " + fname);
  }
  string line;
  while (getline(in, line)) {
    Trim(line);
    if (line.length() == 0 || line[0] == '#')
      continue;
    ep.push_back(line);
    line = "";
  }
  return ep;
}

bool NotURL(const string &p) {
  return p.empty() || (p.substr(0, 5) != "http:" && p.substr(0, 6) != "https:");
}

void Validate(const Config &config) {
  if (config.s3AccessKey.empty() && !config.s3SecretKey.empty() ||
      config.s3SecretKey.empty() && !config.s3AccessKey.empty()) {
    throw invalid_argument(
        "ERROR: both access and secret keys have to be specified");
  }
  if (config.jobs < 1) {
    throw invalid_argument("ERROR: number of jobs must be greater than one, " +
                           to_string(config.jobs) + " provided");
  }
  if (config.maxRetries < 1) {
    throw invalid_argument(
        "ERROR: number of retries must be greater than one, " +
        to_string(config.maxRetries) + " provided");
  }
#ifdef VALIDATE_URL
  const URL url = ParseURL(config.endpoint);
  if (url.proto != "http" && url.proto != "https") {
    throw invalid_argument(
        "ERROR: only 'http' and 'https' protocols supported");
  }
  regex re(R"((\w+\.)*\w+\.\w+)");
  if (!regex_match(url.host, re)) {
    throw invalid_argument("ERROR: invalid endpoint format, should be "
                           "http[s]://hostname[:port]");
  }
  if (url.port > 0xFFFF) {
    throw invalid_argument(
        "ERROR: invalid port number, should be in range[1-65535]");
  }
#endif
}

using Headers = Map;
using Parameters = Map;

atomic<int> numRetriesG{0};

WebClient BuildUploadRequest(const Config &config, const string &path,
                             int partNum, const string &uploadId) {
  Parameters params = {{"partNumber", to_string(partNum + 1)},
                       {"uploadId", uploadId}};
  const string endpoint = config.endpoints[partNum % config.endpoints.size()];
  auto signedHeaders =
      SignHeaders(config.s3AccessKey, config.s3SecretKey, endpoint, "PUT",
                  config.bucket, config.key, "", params);
  Headers headers(begin(signedHeaders), end(signedHeaders));
  WebClient req(endpoint, path, "PUT", params, headers);
  return req;
}

string BuildEndUploadXML(vector<future<string>> &etags) {
  string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
               "<CompleteMultipartUpload "
               "xmlns=\"http://s3.amazonaws.com/doc/2006-03-01/\">\n";
  for (int i = 0; i != etags.size(); ++i) {
    if (!etags[i].valid()) {
      throw runtime_error("Error - request " + to_string(i));
      return "";
    }
    const string part = "<Part><ETag>" + etags[i].get() +
                        "</ETag><PartNumber>" + to_string(i + 1) +
                        "</PartNumber></Part>";
    xml += part;
  }
  xml += "</CompleteMultipartUpload>";
  return xml;
}

WebClient BuildEndUploadRequest(const Config &config, const string &path,
                                vector<future<string>> &etags,
                                const string &uploadId) {
  Parameters params = {{"uploadId", uploadId}};
  const size_t ri = RandomIndex(0, int(config.endpoints.size() - 1));
  const string endpoint = config.endpoints[ri];
  auto signedHeaders =
      SignHeaders(config.s3AccessKey, config.s3SecretKey, endpoint, "POST",
                  config.bucket, config.key, "", params);
  Headers headers(begin(signedHeaders), end(signedHeaders));
  WebClient req(endpoint, path, "POST", params, headers);
  req.SetMethod("POST");
  req.SetPostData(BuildEndUploadXML(etags));
  return req;
}

string UploadPart(const Config &config, const string &path,
                  const string &uploadId, int i, size_t offset,
                  size_t chunkSize, int maxTries = 1, int tryNum = 1) {
  WebClient ul = BuildUploadRequest(config, path, i, uploadId);
  const bool ok = ul.UploadFile(config.file, offset, chunkSize);
  if (!ok) {
    throw(runtime_error("Cannot upload chunk " + to_string(i + 1)));
  }
  const string etag = HTTPHeader(ul.GetHeaderText(), "[Ee][Tt]ag");
  if (etag.empty()) {
    if (tryNum == maxTries) {
      throw(runtime_error("No ETag found in HTTP header"));
    } else {
      numRetriesG += 1;
      return UploadPart(config, path, uploadId, i, offset, chunkSize, maxTries,
                        ++tryNum);
    }
  }
  return etag;
}

void InitConfig(Config &config) {
  if (config.s3AccessKey.empty() && config.s3SecretKey.empty()) {
    const string fname = config.credentials.empty()
                             ? GetHomeDir() + "/.aws/credentials"
                             : config.credentials;
    config.awsProfile =
        config.awsProfile.empty() ? "default" : config.awsProfile;
    Toml toml = ParseTomlFile(fname);
    if (toml.find(config.awsProfile) == toml.end()) {
      throw invalid_argument("ERROR: profile " + config.awsProfile +
                             " not found");
    }
    config.s3AccessKey = toml[config.awsProfile]["aws_access_key_id"];
    config.s3SecretKey = toml[config.awsProfile]["aws_secret_access_key"];
  }
  if (config.endpoint.empty())
    throw invalid_argument("Error, empty endpoint");
  if (NotURL(config.endpoint))
    config.endpoints = ReadEndpoints(config.endpoint);
  else
    config.endpoints.push_back(config.endpoint);
  if (config.endpoints.empty())
    throw invalid_argument("Error, no endpoints specified");
  config.endpoint = config.endpoints[0];
}

//------------------------------------------------------------------------------
int main(int argc, char const *argv[]) {
  try {
    UploadConfig config;
    auto cli =
        lyra::help(config.showHelp).description("Upload file to S3 bucket") |
        lyra::opt(config.s3AccessKey,
                  "awsAccessKey")["-a"]["--access_key"]("AWS access key")
            .optional() |
        lyra::opt(config.s3SecretKey,
                  "awsSecretKey")["-s"]["--secret_key"]("AWS secret key")
            .optional() |
        lyra::opt(config.endpoint,
                  "endpoint")["-e"]["--endpoint"]("Endpoint URL")
            .required() |
        lyra::opt(config.bucket, "bucket")["-b"]["--bucket"]("Bucket name")
            .required() |
        lyra::opt(config.key, "key")["-k"]["--key"]("Key name").required() |
        lyra::opt(config.file, "file")["-f"]["--file"]("File name").required() |
        lyra::opt(config.jobs, "parallel jobs")["-j"]["--jobs"](
            "Number of parallel upload jobs")
            .optional() |
        lyra::opt(config.credentials,
                  "credentials file")["-c"]["--credentials"](
            "Credentials file, AWS cli format")
            .optional() |
        lyra::opt(config.awsProfile, "AWS config profile")["-p"]["--profile"](
            "Profile in AWS config file")
            .optional() |
        lyra::opt(config.maxRetries, "Max retries")["-r"]["--retries"](
            "Max number of per-multipart part retries")
            .optional();

    // Parse the program arguments:
    auto result = cli.parse({argc, argv});
    if (!result) {
      cerr << result.message() << endl;
      cerr << cli << endl;
      exit(1);
    }
    if (config.showHelp) {
      cout << cli;
      return 0;
    }

    InitConfig(config);

    Validate(config);
    // initiate request
    auto signedHeaders =
        SignHeaders(config.s3AccessKey, config.s3SecretKey, endpoint, "POST",
                    config.bucket, config.key, "", {{"uploads", ""}});
    Map headers(begin(signedHeaders), end(signedHeaders));
    WebClient req(endpoint, path, "POST", {{"uploads", ""}}, headers);
    if (!req.Send()) {
      throw runtime_error("Error sending request: " + req.ErrorMsg());
    }
    if (req.StatusCode() >= 400) {
      const string errcode = XMLTag(req.GetContentText(), "[Cc]ode");
      throw runtime_error("Error sending begin upload request - " + errcode);
    }
    vector<uint8_t> resp = req.GetResponseBody();
    const string xml(begin(resp), end(resp));
    const string uploadId = XMLTag(xml, "[Uu]pload[Ii][dD]");
    vector<future<string>> etags(config.jobs);
#ifdef TIME_UPLOAD
    using Clock = chrono::high_resolution_clock;
    auto start = Clock::now();
#endif
    for (int i = 0; i != config.jobs; ++i) {
      const size_t sz = i != config.jobs - 1 ? chunkSize : lastChunkSize;
      etags[i] = async(launch::async, UploadPart, config, path, uploadId, i,
                       chunkSize * i, sz, config.maxRetries, 1);
    }
    WebClient endUpload = BuildEndUploadRequest(config, path, etags, uploadId);
#ifdef TIME_UPLOAD
    const auto end = Clock::now();
    const double elapsed =
        double(
            chrono::duration_cast<chrono::nanoseconds>(end - start).count()) /
        1E9;
    cout << "Elapsed time:  " << elapsed << " s" << endl;
    cout << "Bandwith:      " << std::fixed << std::setprecision(2)
         << (fileSize / double(0x100000)) / elapsed << " MiB/s" << endl;
#endif
    if (!endUpload.Send()) {
      throw runtime_error("Error sending request: " + req.ErrorMsg());
    }
    if (endUpload.StatusCode() >= 400) {
      const string errcode = XMLTag(endUpload.GetContentText(), "[Cc]ode");
      throw runtime_error("Error sending end upload request - " + errcode);
    }
    const string etag = XMLTag(endUpload.GetContentText(), "[Ee][Tt]ag");
    if (etag.empty()) {
      cerr << "Error sending end upload request" << endl;
    }
    cout << etag << endl;
  }
  else {
    auto signedHeaders =
        SignHeaders(config.s3AccessKey, config.s3SecretKey, endpoint, "PUT",
                    config.bucket, config.key, "");
    Map headers(begin(signedHeaders), end(signedHeaders));
    WebClient req(config.endpoint, path, "PUT", {}, headers);
    if (!req.UploadFile(config.file)) {
      throw runtime_error("Error sending request: " + req.ErrorMsg());
    }
    if (req.StatusCode() >= 400) {
      const string errcode = XMLTag(req.GetContentText(), "[Cc]ode");
      throw runtime_error("Error sending upload request - " + errcode);
    }
    const string etag = HTTPHeader(req.GetHeaderText(), "[Ee][Tt]ag");
    if (etag[0] == '"') {
      cout << etag.substr(1, etag.size() - 2) << endl;
    } else {
      cout << etag << endl;
    }
    if (etag.empty()) {
      throw runtime_error("Error sending upload request");
    }
  }
  if (numRetriesG > 0)
    cout << "Num retries: " << numRetriesG << endl;
  return 0;
}
catch (const exception &e) {
  cerr << e.what() << endl;
  return 1;
}
}
namespace sss {
///@todo use size_t
extern int RandomIndex(int, int);
} // namespace sss
void UploadFile(UploadConfig &config) {

  FILE *inputFile = fopen(config.file.c_str(), "rb");
  if (!inputFile) {
    throw runtime_error(string("cannot open file ") + config.file);
  }
  fclose(inputFile);
  // retrieve file size
  const size_t fileSize = FileSize(config.file);
  string path = "/" + config.bucket + "/" + config.key;
  const string endpoint =
      config.endpoints[RandomIndex(0, config.endpoints.size() - 1)];
  if (config.jobs > 1) {
    // compute chunk size
    const size_t chunkSize = fileSize / config.jobs;
    // compute last chunk size
    const size_t lastChunkSize = fileSize % config.jobs == 0
                                     ? chunkSize
                                     : fileSize % config.jobs + chunkSize;
  }
