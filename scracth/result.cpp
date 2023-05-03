// Implementation of Result<Result,Error> type a la Rust.
// author: Ugo Varetto
// License: Zero-clause BSD
// SPDX identifier: 0BSD
#include "result.h"
//-----------------------------------------------------------------------------
Result<int, std::string> Foo(int i) {
  if (i == 0) {
    return Err(std::string("Error"));
  } else {
    return Ok(i);
  }
}

Result<std::reference_wrapper<int>, std::string> FooR(int &i) {
  if (i == 0) {
    return Err(std::string("Error"));
  } else {
    return Ok(std::ref(i));
  }
}
int main(int, char **) {
  int n = 0;
  if (auto r = FooR(n)) {
    // int &rf = (int &)(r);
    std::cout << r << std::endl;
  } else {
    std::cout << Error(r) << std::endl;
  }

  if (auto i = Foo(8)) {
    std::cout << i << std::endl;
    // Exit or exception thrown when accessing error
    // in the presence of a valid result
    // when DISABLE_ERROR_HANDLING NOT #defined
    // stc::cout << Error(i) << std::endl;
  } else {
    // Exit or exception thrown when accessing result
    // in the presence of error
    // when DISABLE_ERROR_HANDLING NOT #defined
    // std::cout << int(i) << std::endl;
    std::cerr << Error(i) << std::endl;
  }
  return 0;
}
