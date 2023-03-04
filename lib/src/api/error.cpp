#include "response_parser.h"
#include "webclient.h"
#include <exception>
#include <string>
using namespace std;

namespace sss {
// Handle error throwing exception
void HandleError(const WebClient &wc, const string &prefix) {
  if (wc.StatusCode() >= 400) {
    const string error = XMLTag(wc.GetContentText(), "Code");
    throw runtime_error(prefix + error);
  }
}
// Handle error throwing exception
void Handle400Error(const WebClient &wc, const string &prefix) {
  if (wc.StatusCode() >= 400) {
    throw runtime_error(prefix + to_string(wc.StatusCode()));
  }
}
} // namespace sss
