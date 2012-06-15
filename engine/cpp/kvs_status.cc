/*************************************************************************
    > File Name: kvs_status.cc
    > Author: takanoter@gmail.com
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kvs_status.h"
namespace kvs {

void Status::SetNotExist() {
/*
    if (NULL == msg_) {
        msg_ = (char*)malloc(STATUS_MSG_SIZE);
    }
    sprintf(msg_, "Not exist.");
*/
    code_ = kNotExist;
}

void Status::SetEndOfFile() {
/*
    if (NULL == msg_) {
        msg_ = new char[STATUS_MSG_SIZE];
    }
    sprintf(msg_, "End of file.");
*/
    code_ = kEndOfFile;
}

void Status::SetExist() {
/*
    if (NULL == msg_) {
        msg_ = new char[STATUS_MSG_SIZE];
    }
    sprintf(msg_, "already exist.");
*/
    code_ = kExist;
}

void Status::SetInvalidParam(const char* name, long value) {
/*
    if (NULL == msg_) {
        msg_ = new char[STATUS_MSG_SIZE];
    }
    sprintf(msg_, "param:%s, value:%ld", name, value);
*/
    code_ = kInvalidParam;
}

void Status::SetIOError(int error_ret) {
/*
    if (NULL == msg_) {
        msg_ = new char[STATUS_MSG_SIZE];
    }
    sprintf(msg_, "error_no:%d", error_ret);
*/
    code_ = kOk;
}

char* Status::CopyState(const char* state) {
    char* result = new char[STATUS_MSG_SIZE];
    memcpy(result, state, STATUS_MSG_SIZE);
    return result;
}

}  // namespace kvs

