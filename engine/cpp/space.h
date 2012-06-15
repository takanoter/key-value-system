/*************************************************************************
    > File Name: space.h
    > Author: takanoter@gmail.com
*/


#ifndef KVS_ENGINE_INCLUDE_SPACE_H_
#define KVS_ENGINE_INCLUDE_SPACE_H_

#include <stdio.h>
#include "kvs_status.h"
#include "kvs_slice.h"
#include "configure.h"

namespace kvs {

const int SPACE_BUFFER_SIZE = 10*1024*1024;
class SPACE {
  public:
    SPACE()  {
    };

    ~SPACE() {
    };
   
    Status Load(CONFIGURE& data_conf);
    Status Born(CONFIGURE& data_conf);
    Status Read(const Offset offset, const Offset length, std::string* value);
    Status Write(const Offset offset, const Offset id, const Slice& key, const Slice& value, const bool sync);
    Offset CalLength(const Slice& key, const Slice& value);

    Offset GetSpace();

  private:
    Offset last_offset_;
    int  fd_;
    //CONFIGURE& conf_;
//    int key_len_;
    int item_head_length_;
    char buffer_[SPACE_BUFFER_SIZE];
 
  private:
    Status SetUse(Offset len);

}; // class SPACE

}; // namespace kvs
#endif // KVS_ENGINE_INCLUDE_SPACE_H_
