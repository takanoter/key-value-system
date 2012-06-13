/*************************************************************************
    > File Name: configure.h
    > Author: takanoter@gmail.com
*/


#ifndef KVS_ENGINE_INCLUDE_CONFIGURE_H_
#define KVS_ENGINE_INCLUDE_CONFIGURE_H_

#include <stdio.h>
#include "kvs_status.h"
#include "kvs_slice.h"

namespace kvs {

const int CONFIGURE_ITEM_SIZE  32
typedef std::map<std::string, std::string> ITEM_MAP;

class CONFIGURE {
  public:
    CONFIGURE() 
        : need_solid(false),
          fd(-1) {
    };

    ~CONFIGURE() {
    };
   
    Status Solid();
    Status Get(const Slice& key, std::string* property);
    Status Set(const Slice& key, const Slice& property);
    Status Load(std::string& pathname);
    Status Create(std::string& pathname);

  private:
    std::string name_;
    bool need_solid_;
    bool blank_;
    int fd_;
    char item_buffer_[CONFIGURE_ITEM_SIZE+1];
    ITEM_MAP items_;
    Offset last_offset_;
}; // class CONFIGURE

}; // namespace kvs
#endif // KVS_ENGINE_INCLUDE_CONFIGURE_H_
