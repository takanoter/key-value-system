/*************************************************************************
    > File Name: item.h
    > Author: takanoter@gmail.com
*/

#ifndef KVS_ENGINE_INCLUDE_ITEM_H_
#define KVS_ENGINE_INCLUDE_ITEM_H_

#include <stdio.h>
#include <stdlib.h>

#include "kvs_status.h"
#include "kvs_slice.h"
#include "posix.h"

namespace kvs {

const int CONFIGURE_ITEM_SIZE=32;
//SOLID black item layout
struct BLACK_ITEM {
    char name[CONFIGURE_ITEM_SIZE - sizeof(Offset)];
    Offset length;
    //really black-data
};

//SOLID visual item layout: name:property
struct VISUAL_ITEM {
    char buf[CONFIGURE_ITEM_SIZE];
};


enum ITEM_TYPE {
    visual = 0, 
    black = 1,
};

/* there are 3 types of item:
 * 1.Visual Item: {name:property}, len=CONFIGURE_ITEM_SIZE
 * 2.Black Item, fix len or synced len: {name,len,data}, name.len+len.len=CONFIGURE_ITEM_SIZE
 * 3.Black Item, non-synced-len(often in EOF: {name,len,data}, len=OffsetFeb31
 */
class ITEM {
 
  public:
    ITEM_TYPE type;
    std::string key;
    std::string value; //需要存实际的值
    Offset len; //when len == OffsetFeb31, means it is meaningless & end of file.
    char *buf;
    int buf_size;
  
  public:
    ITEM() {
    }

    void operator=(const ITEM& s);
  

    ITEM(const Slice& k, const Offset length, char* buffer, int buffer_size) {
        type = black;
        key = k.data();
        len = length;
        buf = buffer;
        buf_size = buffer_size;
    }

    ITEM(const Slice& k, const Slice& v) {
        type = visual;
        key = k.data();
        value = v.data();
        len = 0;
        buf = NULL;
        buf_size = 0;
    }

    ITEM(char* buffer, int buffer_size) {
        int i;
        for (i=0; i<buffer_size; i++) {
            if (':' == buffer[i]) break;
        }
        if (i==buffer_size) {
            //black item
            BLACK_ITEM *black_item = (BLACK_ITEM*) buffer;
            Offset length = black_item->length;
            char* buffer = NULL;
            if (OffsetFeb31 != length) {
                buffer = (char*)malloc((size_t)length);
            }

            type = black;
            key = black_item->name;
            len = length;
            buf = buffer;
            buf_size = buffer_size;
        } else {
            //visual item
            type = visual; 
            key.assign(buffer, i);
            value.assign(&buffer[i+1]);
            len = 0;
            buf = NULL;
            buf_size = buffer_size;
        }
    }

    ~ITEM() {
/*
        if (NULL != buf) {
            free(buf);
        }
*/
    }

    Status Serialize(char* head, const int head_size) {
        assert(head_size >= CONFIGURE_ITEM_SIZE);
        Status s;
        if (type == black) {
            BLACK_ITEM item;
            memset(&item, 0 ,sizeof(item));
            strcpy(&item.name[0], key.data());
            item.length = len;
            memcpy(head, &item, sizeof(item));
        } else {
            char *p = NULL;
            VISUAL_ITEM item;
            memset(&item, 0, sizeof(item));
            snprintf((char*)(&item), sizeof(item), "%s:%s\n", key.data(), value.data());
            memcpy(head, &item, sizeof(item));
        }
        return s;
    }

    bool Fit(const char* item_head, const int head_size) {
        if (strncmp(key.data(), item_head, key.size()) == 0) {
            return true;
        }
        return false;
    }
}; // class ITEM

inline void ITEM::operator=(const ITEM& item) {
    type = item.type;
    key = item.key;
    value = item.value;
    len = item.len;
    buf = item.buf;
    buf_size = item.buf_size;
}

}; // namespace kvs

#endif // KVS_ENGINE_INCLUDE_ITEM_H_
