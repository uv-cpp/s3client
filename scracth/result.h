// Implementation of Result<Result,Error> type a la Rust.
// author: Ugo Varetto
// License: Zero-clause BSD
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
template <typename T> auto Err(T &&e) { return std::move(ErrorType(e)); }

// Error type requires to_string overload
inline std::string to_string(const std::string &s) { return s; }

/// @todo specialize, only supported for non ref types
template <typename T> struct OkType {
  T t_;
  OkType(T t) : t_(t) {}
};

template <typename T> OkType<T> Ok(T t) { return OkType(t); }

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
// This toy implementation supports references by using reference_wrapper<>
// in unions.
// const &, & and && are all supported for basic cases. volatile not handled
// explicitly.
// When DISABLE_ERROR_HANDLING is NOT #defined
// the default behaviour in case of access to result in the presence of error is
// exiting; can throw exception instead.

//=============================================================================
template <typename ErrT> struct ErrorHandler {
  bool ok_ = false;
  const ErrT &error_;
#ifndef DISABLE_ERROR_HANDLING
  void ExitIfError(bool ok, const std::string &msg) const {
    if (ok_ != ok) {
      std::cerr << msg << " - " << to_string(error_) << std::endl;
      exit(EXIT_FAILURE);
    }
  }
  void ThrowIfError(bool ok, const std::string &msg) const {
    if (ok_ != ok) {
      throw std::logic_error(msg + " - " + to_string(error_));
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
  ErrorHandler(bool ok, const ErrT &err) : ok_(ok), error_(err) {}
  bool Ok() const { return ok_; }
};

//=============================================================================
// Result implementation
template <typename R, typename ErrT> class Result : private ErrorHandler<ErrT> {
  using Base = ErrorHandler<ErrT>;
  friend const ErrT &Error(const Result &r) {
#ifndef DISABLE_ERROR_HANDLING
    r.HandleError(false); // if ok != false segfaults
#endif
    return r.error_;
  }
  friend const R &Value(const Result &r) {
#ifndef DISABLE_ERROR_HANDLING
    r.HandleError(); // if ok != false segfaults
#endif
    return r.result_;
  }
  friend R &Value(Result &r) {
#ifndef DISABLE_ERROR_HANDLING
    r.HandleError(); // if ok != false segfaults
#endif
    return r.result_;
  }
  friend R &&Value(Result &&r) {
#ifndef DISABLE_ERROR_HANDLING
    r.HandleError(); // if ok != false segfaults
#endif
    return std::move(r.result_);
  }

private:
  //  union cannot have rerence type, either replace with
  //  struct increasing the size or wrap references
  //  with ref/cref
  union {
    R result_;
    ErrT error_;
  };

public:
  Result(Result &&r) : Base(r.ok_, error_) {
    if (r.Ok()) {
      result_ = std::move(r.result_);
    } else {
      error_ = std::move(r.error_);
    }
  }
  Result(const Result &r) : Base(r.ok_, error_) {
    if (r.Ok()) {
      result_ = r.result_;
    } else {
      error_ = r.error_;
    }
  }
  ~Result() {
    if (Base::Ok()) {
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
  Result(OkType<R> t) : Base(true, error_), result_(t.t_) {}
  Result(ErrorType<ErrT> &&err)
      : Base(false, error_), error_(std::move(err.err)) {}
  Result(const R &r) : Base(true, error_), result_(r) {}
  Result(R &&r) : Base(true, error_), result_(std::move(r)) {}
  operator bool() const { return Base::Ok(); }
  operator const R &() const {
#ifndef DISABLE_ERROR_HANDLING
    Base::HandleError();
#endif
    return result_;
  }
  operator R &&() {
#ifndef DISABLE_ERROR_HANDLING
    Base::HandleError();
#endif
    return std::move(result_);
  }
};
//=============================================================================
//-----------------------------------------------------------------------------
// Specialization for const reference -> reference_wrapper<const T>
template <typename R, typename ErrT>
class Result<const R &, ErrT> : private ErrorHandler<ErrT> {
  using Base = ErrorHandler<ErrT>;
  friend const ErrT &Error(const Result &r) {
#ifndef DISABLE_ERROR_HANDLING
    r.HandleError(false); // if ok != false segfaults
#endif
    return r.error_;
  }
  friend const R &Value(const Result &r) {
#ifndef DISABLE_ERROR_HANDLING
    r.HandleError(); // if ok != false segfaults
#endif
    return r.result_;
  }

private:
  union {
    std::reference_wrapper<const R> result_;
    ErrT error_;
  };

public:
  Result(Result &&r) : Base(r.Ok(), error_) {
    if (r.Ok()) {
      result_ = std::move(r.result_);
    } else {
      error_ = std::move(r.error_);
    }
  }
  Result(const Result &r) : Base(r.Ok(), error_) {
    if (r.Ok()) {
      result_ = r.result_;
    } else {
      error_ = r.error_;
    }
  }
  ~Result() {
    if (Base::Ok()) {
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
  Result(ErrorType<ErrT> &&err)
      : Base(false, error_), error_(std::move(err.err)) {}
  Result(const R &r) : Base(true, error_), result_(r) {}
  Result(R &&r) : Base(true, error_), result_(std::move(r)) {}
  operator bool() const { return Base::Ok(); }
  operator const R &() const {
#ifndef DISABLE_ERROR_HANDLING
    Base::HandleError();
#endif
    return result_;
  }
  operator const R &() {
#ifndef DISABLE_ERROR_HANDLING
    Base::HandleError();
#endif
    return result_;
  }
};
//=============================================================================
//-----------------------------------------------------------------------------
// specialization for reference -> reference_wrapper<T>
template <typename R, typename ErrT>
class Result<R &, ErrT> : private ErrorHandler<ErrT> {
  using Base = ErrorHandler<ErrT>;
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
  friend R &Value(const Result &r) {
#ifndef DISABLE_ERROR_HANDLING
    r.HandleError(); // if ok != false segfaults
#endif
    return r.result_;
  }

private:
  union {
    std::reference_wrapper<R> result_;
    ErrT error_;
  };

public:
  Result(Result &&r) : Base(r.Ok()) {
    if (r.Ok()) {
      result_ = std::move(r.result_);
    } else {
      error_ = std::move(r.error_);
    }
  }
  Result(const Result &r) : Base(r.Ok()) {
    if (r.Ok()) {
      result_ = r.result_;
    } else {
      error_ = r.error_;
    }
  }
  ~Result() {
    if (Base::Ok()) {
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
  Result(ErrorType<ErrT> &&err)
      : Base(false, error_), error_(std::move(err.err)) {}
  Result(R &r) : Base(true, error_), result_(r) {}
  operator bool() const { return Base::Ok(); }
  operator const R &() const {
#ifndef DISABLE_ERROR_HANDLING
    Base::HandleError();
#endif
    return result_;
  }
  operator R &() {
#ifndef DISABLE_ERROR_HANDLING
    Base::HandleError();
#endif
    return result_;
  }
};
