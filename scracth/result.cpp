// Implementation of Result<Result,Error> type a la Rust.
// author: Ugo Varetto
// SPDX identifier: 0BSD

#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>

template <typename T> struct ErrorType {
  T err;
  ErrorType() = delete;
  ErrorType(const T &e) : err(e) {}
  ErrorType(T &&e) : err(std::move(e)) {}
};
template <typename T> struct OkType {
  T r;
  OkType() = delete;
  OkType(const T &r) : r(r) {}
  OkType(T &&r) : r(std::move(r)) {}
};

template <typename T> auto Err(T &&e) { return std::move(ErrorType(e)); }

template <typename T> auto Ok(T r) { return OkType(r); }
//-----------------------------------------------------------------------------
// A union can have member functions (including constructors and destructors),
// but not virtual (10.3) functions. A union shall not have base classes. A
// union shall not be used as a base class. If a union contains a non-static
// data member of reference type the program is ill-formed. At most one
// non-static data member of a union may have a brace-or-equal-initializer . [
// Note: If any non-static data member of a union has a non-trivial default
// constructor (12.1), copy constructor (12.8), move constructor (12.8), copy
// assignment operator (12.8), move assignment operator (12.8), or destructor
// (12.4), the corresponding member function of the union must be user-provided
// or it will be implicitly deleted (8.4.3) for the union. — end note ]
// i.e. A CUSTOM DESTRUCTOR TO CLEAN UP UNION ELEMENTS MUST BE PROVIDED
//
// A program is ill-formed if it instantiates an expected with a reference type,
// a function type, or a specialization of std::unexpected. In addition, T must
// not be std::in_place_t or std::unexpect_t i.e. C++23 DOES NOT SUPPORT
// expected WITH REFERENCES!
//
// When DISABLE_ERROR_HANDLING is NOT #defined the default behaviour in case of
// access to result in the presence of error is exiting; can throw exception
// instead.
template <typename R, typename ErrT> class Result {
  template <typename R2, typename E2>
  friend Result<R2, E2> ResultCast(const Result<R, ErrT> &);
  template <typename R2, typename E2>
  friend Result<R2, E2> ResultCast(Result<R, ErrT> &&);
  friend const ErrT &Error(const Result &r) {
#ifndef DISABLE_ERROR_HANDLING
    r.HandleError(false); // if ok != false segfaults
#endif
    return r.error_;
  }

private:
  bool ok_ = false;
  // union cannot have rerence type, either replace with
  // struct increasing the size or wrap references
  // with ref/cref; note: when using struct reference_wrapper
  // cannot be used anymore
  union {
    R result_;
    ErrT error_;
  };
#ifndef DISABLE_ERROR_HANDLING
  void ExitIfError(bool ok, const std::string &msg) const {
    if (ok_ != ok) {
      std::cerr << msg << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  void ThrowIfError(bool ok, const std::string &msg) const {
    if (ok_ != ok) {
      throw std::logic_error(msg);
    }
  }

  void HandleError(bool ok = true,
                   const std::string &msg = "Unchecked error condition") const {
#ifdef THROW_IF_ERROR
    ThrowIfError(ok, msg);

#else
    ExitIfError(ok, msg);
#endif
  }
#endif

public:
  Result(Result &&r) : ok_(r.ok_) {
    if (r.ok_) {
      result_ = std::move(r.result_);
    } else {
      error_ = std::move(r.error_);
    }
  }
  Result(const Result &r) : ok_(r.ok_) {
    if (r.ok_) {
      result_ = r.result_;
    } else {
      error_ = r.error_;
    }
  }
  ~Result() {
    if (ok_) {
      if constexpr (!std::is_trivially_destructible<R>::value) {
        result_.~R();
      }
    } else {
      if constexpr (!std::is_trivially_destructible<ErrT>::value) {
        error_.~ErrT();
      }
    }
  }
  Result() = delete;
  Result(ErrorType<ErrT> &&err) : ok_(false), error_(std::move(err.err)) {}
  Result(OkType<R> &&res) : ok_(true), result_(std::move(res.r)) {}
  Result(const R &r) : ok_(true), result_(r) {}
  Result(R &&r) : ok_(true), result_(std::move(r)) {}
  operator bool() const { return ok_; }
  operator const R &() const {
#ifndef DISABLE_ERROR_HANDLING
    HandleError();
#endif
    return result_;
  }
  operator R &() {
#ifndef DISABLE_ERROR_HANDLING
    HandleError();
#endif
    return result_;
  }
};

template <typename R1, typename E1, typename R2, typename E2>
Result<R1, E2> ResultCast(const Result<R1, E1> &r1) {
  if (r1) {
    return {r1.result_};
  } else {
    return {r1.error_};
  }
}

template <typename R1, typename E1, typename R2, typename E2>
Result<R1, E2> ResultCast(Result<R1, E1> &&r1) {
  if (r1) {
    return {std::move(r1.result_)};
  } else {
    return {std::move(r1.error_)};
  }
}

#ifdef TEST
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
    std::cout << int(i) << std::endl;
    std::cerr << Error(i) << std::endl;
  }
  return 0;
}
#endif
