/*************************************************************************
    > File Name: configure.h
    > Author: takanoter@gmail.com
*/


#ifndef KVS_ENGINE_INCLUDE_CONFIGURE_H_
#define KVS_ENGINE_INCLUDE_CONFIGURE_H_

#include <stdio.h>
#include <map>
#include "kvs_status.h"
#include "kvs_slice.h"
#include "item.h"

namespace kvs {

typedef std::map<std::string, ITEM> ITEM_MAP;

class CONFIGURE {
  public:
    CONFIGURE() 
        : need_solid_(false),
          fd_(-1) {
    };

    ~CONFIGURE() {
    };
   
    Status Solid();
    Status Get(const Slice& key, std::string* property); 
    Status Set(const Slice& key, const Slice& property);
    Status NewItem(const Slice& key, const Offset len);
    Status NewItem(const Slice& key); 
    Status NewItem(const Slice& key, const Slice& value);
    char*  GetBuffer(const Slice& key);
    Status Load(const std::string& pathname);
    Status Create(const std::string& pathname);
    Status FetchLastOffset(Offset *off);
    int const GetFD();
    Status Reborn(CONFIGURE& base_conf);

  private:
    ITEM_MAP items_;
    std::string name_;
    Offset last_offset_;
    bool need_solid_;
    bool blank_;
    int  fd_;

    //buffer
    char item_buffer_[CONFIGURE_ITEM_SIZE+1];
 
  private:
    Status NewItem(const ITEM& item);
    Status Set(const ITEM& item);
    Status AppendItem(ITEM& item, Offset *offset);
    Status InjectItem(ITEM& item);
    void   NextOffset(const ITEM& item, Offset *offset);
    Status FetchItemBuffer(const Offset off);
    Status SearchItemOffset(ITEM& item, Offset* offset);
    ITEM_MAP::iterator ITEM_MAP_FIND(ITEM_MAP& items_map, std::string& key);
//    Status GetItemOffsetBlack(const Slice& key, Offset* offset); 
}; // class CONFIGURE

}; // namespace kvs
#endif // KVS_ENGINE_INCLUDE_CONFIGURE_H_
