#include "response_parser.h"
#include "webclient.h"
#include <exception>
#include <string>
using namespace std;

namespace sss {
namespace api {
// Handle error throwing exception
void HandleError(const WebClient &wc, const string &prefix) {
  if (wc.StatusCode() >= 400) {
    const string error = XMLTag(wc.GetContentText(), "Code");
    throw runtime_error(prefix + error);
  }
}
} // namespace api
} // namespace sss
