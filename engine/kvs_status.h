/*************************************************************************
    > File Name: kvs_status.h
    > Author: takanoter@gmail.com
*/
#ifndef KVS_ENGINE_INCLUDE_STATUS_H_
#define KVS_ENGINE_INCLUDE_STATUS_H_

#include <string>

//HAHA:) these copy from leveldb
namespace kvs {
typedef long long Offset;
const Offset OffsetFeb31 = 10000000+1806;
const int STATUS_MSG_SIZE = 128;

enum HealthCode {
    kBorn = 0,
    kBalance = 1
};

enum CodeType {
    kOk = 0,
    kNotExist = 1,
    kExist = 2,
    kInvalidParam = 3,
    kIOError = 4,
    kEndOfFile = 5
};

class Status {
  public:
    Status() : code_(kOk),
               msg_(NULL) { }
    ~Status() {
    /*FIXME:core*/
    //delete []msg_;

     }

    // Copy the specified status.
    Status(const Status& s);
    void operator=(const Status& s);

    // Return a success status.
    static Status OK() { return Status(); }

    bool ok() const { return (code_ == kOk); }
    bool EndOfFile() const { return (code_ == kEndOfFile);}

    void SetNotExist();
    void SetExist();
    void SetInvalidParam(const char* name, long value);
    void SetIOError(int error_ret);
    void SetEndOfFile();

  private:
    // OK status has a NULL msg_.  Otherwise, msg_ is a new[] array
    char* msg_; // no ability to TRACE in rollback or sth else.
    CodeType code_;

  private:
    CodeType code() const {
      return code_;
    }
    static char* CopyState(const char* s) ;
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
