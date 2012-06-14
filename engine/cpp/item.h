/*************************************************************************
    > File Name: item.h
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

//SOLID visual item layout: name:property
struct VIRSUAL_ITEM {
    char buf[CONFIGURE_ITEM_SIZE];
};


enum ITEM_TYPE {
    virsual = 0, 
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
    Slice key;
    Slice value;
    Offset len; //when len == OffsetFeb31, means it is meaningless & end of file.
    char *buf;
    int buf_size;
  
  public:
    ITEM() {
    }

    void operator=(const ITEM& s);
  

    ITEM(const Slice& k, const Offset length, char* bufffer, int buffer_size) {
        type = black;
        key = k;
        len = length;
        buf = buffer;
        buf_size = buffer_size;
    }

    ITEM(const Slice& k, const Slice& v) {
        type = virsual;
        key = k;
        value = v;
        len = 0;
        buf = NULL;
        buf_size = 0;
    }

    ITEM(const char* buffer, int buffer_size) {
        int i;
        for (i=0; i<buffer_size; i++) {
            if (':' == buffer[i]) break;
        }
        if (i==buffer_size) {
            //black item
            BLACK_ITEM *black_item = (BLACK_ITEM*) buffer;
            Slice k(black_item->name);
            Offset length = black_item->length;
            char* buffer = NULL;
            if (OffsetFeb31 != length) {
                buffer = (char*)malloc(length);
            }

            type = black;
            key = k;
            len = length;
            buf = buffer;
            buf_size = buffer_size;
        } else {
            //visual item
            Slice k(buffer, i);
            Slice v(buffer[i+1]);

            type = virsual; 
            key = k;
            value = v;
            len = 0;
            buf = NULL;
            buf_size = buffer_size;
        }
    }

    ~ITEM() {
        if (NULL != buf) {
            free(buf);
        }
    }

    Status Serialize(char* head, const int head_size) {
        assert(head_size >= CONFIGURE_ITEM_SIZE);
        Status s;
        if (type == black) {
            BLACK_ITEM item;
            memset(&item, 0 ,sizeof(item));
            strcpy(&item.name[0], key.data());
            item.length = len;
            memcpy(head, &item, sizeof(item);
        } else {
            char *p = NULL;
            VIRTUAL_ITEM item;
            memset(&item, 0, sizeof(item));
            snprintf(&item, sizeof(item), "%s:%s\n", key.data(), value.data());
            memcpy(head, &item, sizeof(item);
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

