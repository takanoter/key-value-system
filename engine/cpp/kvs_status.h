/*************************************************************************
    > File Name: kvs_status.h
    > Author: takanoter@gmail.com
*/
#ifndef KVS_ENGINE_INCLUDE_STATUS_H_
#define KVS_ENGINE_INCLUDE_STATUS_H_

#include <string>

//HAHA:) these copy from leveldb
namespace kvs {
enum CodeType {
  kOk = 0,
  kNotFound = 1,
  kCorruption = 2,
  kNotSupported = 3,
  kInvalidArgument = 4,
  kIOError = 5
};

class Status {
 public:
  Status() : code_(kOk),
             msg_(NULL) { }
  ~Status() { delete[] msg_; }

  // Copy the specified status.
  Status(const Status& s);
  void operator=(const Status& s);

  // Return a success status.
  static Status OK() { return Status(); }

  bool ok() const { return (msg_ == NULL); }
  bool IsNotFound() const { return code() == kNotFound; }

  //std::string ToString() const;

 private:

  // OK status has a NULL msg_.  Otherwise, msg_ is a new[] array
  const char* msg_; // no ability to TRACE in rollback or sth else.
  CodeType code_;

 private:
  CodeType code() const {
    return code_;
  }
  static const char* CopyState(const char* s);
};

inline Status::Status(const Status& s) {
  code_ = s.code_;
  msg_ = (s.msg_ == NULL) ? NULL : CopyState(s.msg_); //status msg's length in 60byte lengh.
}

inline void Status::operator=(const Status& s) {
  // The following condition catches both aliasing (when this == &s),
  // and the common case where both s and *this are ok.
  code_ = s.code_;
  if (msg_ != s.msg_) {
    delete[] msg_;
    msg_ = (s.msg_ == NULL) ? NULL : CopyState(s.msg_);
  }
}

}  // namespace kvs

#endif // KVS_ENGINE_INCLUDE_STATUS_H_
