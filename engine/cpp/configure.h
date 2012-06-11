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
    std::string name;
    bool need_solid;
    int fd;
}; // class CONFIGURE

}; // namespace kvs
#endif // KVS_ENGINE_INCLUDE_CONFIGURE_H_
