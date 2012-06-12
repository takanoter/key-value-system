/*************************************************************************
    > File Name: configure.cc
    > Author: takanoter@gmail.com
*/

#include <stdio.h>
#include "kvs_status.h"
#include "kvs_slice.h"
#include "configure.h"
#include "posix.h"

namespace kvs {

//SOLID black item layout
struct BLACK_ITEM {
    char name[CONFIGURE_ITEM_SIZE - sizeof(Offset)];
    Offset length;
    //really black-data
};

//SOLID visual item layout
struct VISUAL_ITEM {
    char name[CONFIGURE_ITEM_SIZE];
};


enum ITEM_TYPE {
    virsual = 0, 
    black = 1,
};

/* there are 3 types of item:
 * 1.Visual Item: {name:property}, len=CONFIGURE_ITEM_SIZE
 * 2.Black Item, fix len or synced len: {name,len,data}, name.len+len.len=CONFIGURE_ITEM_SIZE
 * 3.Black Item, non-synced-len(often in EOF: {name,len,data}, len=Offset.NotExist
 */
struct ITEM {
    ITEM_TYPE type;
    Slice key;
    Slice value;
    Offset len; //when len == Offset.NotExist, means it is meaningless & end of file.
    char *buf;
};//in memory structure;

Status CONFIGURE::Solid() {
    Status s;
    if (blank_) {
        //遍历 {
           s = AppendItem(item, &off);
        }
        s = AppendBlackSign(&off);
        //遍历Black {
           s = AppendItem(item, &off);
        }
        blank_ = false;
    } else {
        // 遍历 {
            s = InjectItem(item);
        }
        // 遍历 {
            s = InjectItem(item);
        }
    }
    need_solid_ = false;
    return s;
}

//Memory&Visual Only
Status CONFIGURE::Get(const Slice& key, std::string* property) {

}

//Memory&Visual Only
Status CONFIGURE::Set(const Slice& key, const Slice& property) {

}

Status CONFIGURE::SetBlack(const Slice& key, const Offset len, const char *buf_p, const bool fix) {



} 

//Memory Only
Offset CONFIGURE::GetLastOffset() {
    return last_offset;
}


Status CONFIGURE::Load(std::string& pathname) {
    name_ = pathname;
    Status s = LoadFile(pathname, &fd_);
    if (!s.ok()) return s;
    memset(item_buffer_, 0, sizeof(item_buffer_));

    Offset off = 0;
    while (!s.EndOfFile()) {
        Item item;
        s = FetchItem(off, &item);
        if (!s.ok()) {
            if (s.EndOfFile()) break;
            return s;
        }
        s = SetProperty(item);
        if (!s.ok()) return s;
        NextOffset(item, &off);
        if (Offset.NotExist == off) break;
    }

    s = FetchLastOffset(&last_offset_);
    if (!s.ok()) return s;
    need_solid_ = false;
    blank_ = false;
    return s;
}

Status CONFIGURE::Create(std::string& pathname) {
   name_ = pathname;
   Status s = CreateFile(pathname, &fd_);
   if (!s.ok()) return s;
   memset(item_buffer_, 0, sizeof(item_buffer_));

   last_offset_ = Off.NotExist;
   need_solid_ = true;
   blank_ = true;
   return s;
}

Status CONFIGURE::FetchLastOffset(Offset *off) {
    //Linux 返回文件最后的offset
}

Status CONFIGURE::FetchItem(Offset off, ITEM *item)
{
    Offset off = 0;
    Status s;
    




}

/*
Status CONFIGURE::GetItemOffset(const Slice& key, Offset* offset) {
    Offset off = 0; 
    Status s;
    while (!s.EndOfFile()) {
        s = ReadFile(fd, off, item_buffer, CONFIGURE_ITEM_SIZE);
        if (!s.ok()) {
            if (s.EndOfFile()) {
                break;
            }
            return s;
        }
        if (strncmp(key.data(), item_buffer, key.size()) == 0) {
            *offset = off;
            return s;
        }
        off += CONFIGURE_ITEM_SIZE;
    }
    s.SetNotExist();
    return s;
}

Status CONFIGURE::GetItemOffsetBlack(const Slice& key, Offset* offset) {
     Status s = GetItemOffset(key, offset);
     *offset += CONFIGURE_ITEM_SIZE; 
     return s;
};
*/
}; // namespace kvs

