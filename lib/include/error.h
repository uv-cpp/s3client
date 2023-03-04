#pragma once
#include "webclient.h"
#include <string>
namespace sss {
void Handle400Error(const WebClient &wc, const std::string &prefix = "");
void HandleError(const WebClient &wc, const std::string &prefix = "");
} // namespace sss
