/*************************************************************************
    > File Name: posix.h
    > Author: takanoter@gmail.com
*/
#ifndef KVS_ENGINE_INCLUDE_POSIX_H_
#define KVS_ENGINE_INCLUDE_POSIX_H_

#include <stdio.h>
#include "kvs_status.h"
#include "kvs_slice.h"

namespace kvs {

Status CreateFile(const std::string pathname, int *fd);
Status LoadFile(const std::string pathname, int *fd);
Status WriteFile(const int fd, const Offset offset, const Slice& slice);
Status ReadFile(const int fd, const Offset offset, char* buffer, const int length);

}; // namespace kvs

#endif // KVS_ENGINE_INCLUDE_POSIX_H_
