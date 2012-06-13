/*************************************************************************
    > File Name: posix.h
    > Author: takanoter@gmail.com
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "posix.h"

namespace kvs {

Status CreateFile(const std::string pathname, int *fd) {
    Status s;
    *fd = open(pathname.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if (*fd < 0) {
        s.SetIOError();
    }
    return s;
}

Status LoadFile(const std::string pathname, int *fd) {
    Status s;
    *fd = open(pathname.c_str(), O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if (*fd < 0) {
        s.SetIOError(*fd);
    }
    return s;
}

Status WriteFile(const int fd, const Offset offset, const char* buf, const int len) {
    int ret = 0;
    Status s;
    /*FIXME*/
    ret = lseek(fd, offset, SEEK_SET);
    ret = write(fd, buf, len);
    if (ret<0) {
        s.SetIOError(ret);
    }
    return s;
}
Status WriteFile(const int fd, const Offset offset, const Slice& slice) {
    int ret = 0;
    Status s;
    /*FIXME*/
    ret = lseek(fd, offset, SEEK_SET);
    ret = write(fd, slice.data(), slice.size());
    if (ret<0) {
        s.SetIOError(ret);
    }
    return s;
}

Status ReadFile(const int fd, const Offset offset, char* buf, const int len) {
    int ret = 0;
    Status s ;
    /*FIXME*/
    ret = lseek(fd, offset, SEEK_SET);
    ret = read(fd, buf, len);
    if (ret<0) {
        s.SetIOError(ret);
    }
    return s;
}

Status GetFileLast(const int fd, Offset *offset)
{
    Status s;
    ret = lseek(fd, 0, SEEK_SET);
    off = lseek(fd, 0, SEEK_END);
    *offset = off;
    return s;
}

}; // namespace kvs

