/*************************************************************************
    > File Name: configure.cc
    > Author: takanoter@gmail.com
*/

#include <stdio.h>
#include <list>
#include "kvs_status.h"
#include "kvs_slice.h"
#include "item.h"
#include "configure.h"
#include "posix.h"

namespace kvs {

//Memory&Visual Only
Status CONFIGURE::Get(const Slice& key, std::string* property) {
    Status s;
    std::string k = key.data();
    ITEM_MAP::iterator it = ITEM_MAP_FIND(items_, k);
    ITEM item;
    if (it == items_.end()) {
        s.SetNotExist();
    } else {
        item = it->second;;
        *property = item.value.data();
    }
    return s;
}

//Memory&Visual Only
Status CONFIGURE::Set(const Slice& key, const Slice& property) {
    Status s;
    std::string k = key.data();
    ITEM_MAP::iterator it = ITEM_MAP_FIND(items_, k);
    if (it == items_.end()) {
        s.SetNotExist();
    } else {
        ITEM item(key, property);
        items_.erase(it);
        items_.insert(ITEM_MAP::value_type(k, item));
        need_solid_ = true;
    }
    return s;
}
/*
    ITEM_MAP::iterator it;
    it = items_.find(key.data());
    if (items_.end() == it) return NULL;
    return it->second.buf;
    c++ std::map不是覆盖插入的。
*/

Status CONFIGURE::Set(const ITEM& item) {
    Status s;
    std::string k = item.key.data();
    ITEM_MAP::iterator it = ITEM_MAP_FIND(items_, k);
    if (items_.end() != it) items_.erase(it);
    items_.insert(ITEM_MAP::value_type(k, item));
    need_solid_ = true;
    return s;
}

//malloc in here
Status CONFIGURE::NewItem(const Slice& key, const Offset len) {
    Status s;
    char* buf = (char*) malloc(len);
    //printf ("%s--%lld\n", key.data(), len);
    ITEM item(key, len, buf, len);
    std::string k = key.data();
    ITEM_MAP::iterator it = ITEM_MAP_FIND(items_, k);
    if (items_.end() != it) items_.erase(it);
    items_.insert(ITEM_MAP::value_type(k, item));
    return s;
} 

Status CONFIGURE::NewItem(const Slice& key) {
    Status s;
    ITEM item(key, OffsetFeb31, NULL, 0);
    std::string k = key.data();
    ITEM_MAP::iterator it = ITEM_MAP_FIND(items_, k);
    if (items_.end() != it) items_.erase(it);
    items_.insert(ITEM_MAP::value_type(k, item));
    return s;
}

Status CONFIGURE::NewItem(const Slice& key, const Slice& value) {
    Status s;
    ITEM item(key, value);
    std::string k = key.data();
    ITEM_MAP::iterator it = ITEM_MAP_FIND(items_, k);
    if (items_.end() != it) items_.erase(it);
    items_.insert(ITEM_MAP::value_type(k, item)); 
    return s;
}

Status CONFIGURE::NewItem(const ITEM& item)
{
    Status s;
    std::string k = item.key.data(); 
    ITEM_MAP::iterator it = ITEM_MAP_FIND(items_, k);
    if (items_.end() != it) items_.erase(it);
    items_.insert(ITEM_MAP::value_type(k, item)); 
    return s;
}

char* CONFIGURE::GetBuffer(const Slice& key) {
    std::string k = key.data();
    ITEM_MAP::iterator it = ITEM_MAP_FIND(items_, k);
    if (items_.end() == it) return NULL;
    return it->second.buf;
}


//Memory Only, ignore..
/*
Offset CONFIGURE::GetLastOffset() {
    return last_offset;
}
*/

Status CONFIGURE::Solid() {
    Status s;
    ITEM_MAP::iterator it;
    if (blank_) {
        Offset off = 0;
        std::list<ITEM> black_items;
        std::list<ITEM> last_items;
        for (it = items_.begin(); it != items_.end(); it++) {
            ITEM item = it->second;
            if (item.type == black) {
                if (OffsetFeb31 == item.len) {
                    last_items.push_back(item);
                } else {
                    black_items.push_back(item);
                }
            } else {
                //printf ("blank solid:[%s:%lld]\n", it->first.c_str(), off); 
                s = AppendItem(it->second, &off);
            }
        }

        std::list<ITEM>::iterator iter;
        for (iter=black_items.begin(); iter!=black_items.end(); iter++) {
            //printf ("blank solid:[%s:%lld]\n", iter->key.c_str(), off); 
            s = AppendItem(*iter, &off);
        }
        for (iter=last_items.begin(); iter!=last_items.end(); iter++) {
            //printf ("blank solid:[%s:%lld]\n", iter->key.c_str(), off); 
            s = AppendItem(*iter, &off);
        }

        s = FetchLastOffset(&last_offset_);
        if (!s.ok()) {
           last_offset_ = OffsetFeb31;  
           return s;
        }
        blank_ = false;
    } else {
        for (it = items_.begin(); it != items_.end(); it++) {
            s = InjectItem(it->second);
            if (!s.ok()) {
                return s;
            }
        }
    }
    need_solid_ = false;
    return s;
}

Status CONFIGURE::Load(const std::string& pathname) {
    name_ = pathname;
    Status s = LoadFile(pathname, &fd_);
    if (!s.ok()) return s;
    memset(item_buffer_, 0, sizeof(item_buffer_));

    Offset off = 0;
    while (!s.EndOfFile()) {
        //offset is always before CONFIGURE_ITEM
        s = FetchItemBuffer(off);
        if (!s.ok()) {
            if (s.EndOfFile()) break;
            return s;
        }
        ITEM item(item_buffer_, CONFIGURE_ITEM_SIZE);
        //printf ("-- load item trace [%s:%s:%lld]\n", item.key.c_str(), item.value.c_str(), item.len);
        if (item.buf != NULL) {
            s = ReadFile(fd_, off+CONFIGURE_ITEM_SIZE, item.buf, item.len);
            if (!s.ok()) return s;
        }
        
        s = NewItem(item);
        if (!s.ok()) return s;
        NextOffset(item, &off);
        if (OffsetFeb31 == off) break;
    }

    s = FetchLastOffset(&last_offset_);
    if (!s.ok()) return s;
    need_solid_ = false;
    blank_ = false;
    return s;
}

Status CONFIGURE::Create(const std::string& pathname) {
   name_ = pathname;
   Status s = CreateFile(pathname, &fd_);
   if (!s.ok()) return s;
   memset(item_buffer_, 0, sizeof(item_buffer_));

   last_offset_ = OffsetFeb31;
   need_solid_ = true;
   blank_ = true;
   return s;
}

/*****************  private  *********************************/
Status CONFIGURE::AppendItem(ITEM& item, Offset *offset) {
    Status s;
    
    item.Serialize(item_buffer_, CONFIGURE_ITEM_SIZE);
    s = WriteFile(fd_, *offset, (const char*)item_buffer_, (const int)CONFIGURE_ITEM_SIZE);
    if (!s.ok()) return s;
    if (black == item.type && OffsetFeb31 != item.len) {
        s = WriteFile(fd_, *offset + CONFIGURE_ITEM_SIZE,  (const char*)item.buf, (const int)item.len);
        if (!s.ok()) return s;
        *offset += item.len;
    }
    
    //do it at last for rollback
    *offset += CONFIGURE_ITEM_SIZE;
    return s;
}

Status CONFIGURE::InjectItem(ITEM& item) {
    Status s;
    Offset off = 0;
    s = SearchItemOffset(item, &off);
    //printf ("[inject %s:%lld]\n\n", item.key.c_str(), off);
    if (!s.ok()) {
        return s;
    }
    s = AppendItem(item, &off);
    if (!s.ok()) return s;
    return s;
}

//go to head
void CONFIGURE::NextOffset(const ITEM& item, Offset *offset) {
    *offset += CONFIGURE_ITEM_SIZE;
    if (item.type == black)  *offset += item.len;
    if (OffsetFeb31 == item.len) *offset = OffsetFeb31;
    return;
}

Status CONFIGURE::FetchLastOffset(Offset *off) {
    //Linux 返回文件最后的offset
    return GetFileLast(fd_, off);
}

Status CONFIGURE::FetchItemBuffer(const Offset off)
{
    Status s;
    memset(item_buffer_, 0, sizeof(item_buffer_));
    s = ReadFile(fd_, off, item_buffer_, CONFIGURE_ITEM_SIZE);
    if (!s.ok()) return s;
    return s;
}


Status CONFIGURE::SearchItemOffset(ITEM& item, Offset* offset) {
    Offset off = 0; 
    ITEM new_item;
    Status s;
    while (!s.EndOfFile()) {
        s = ReadFile(fd_, off, item_buffer_, CONFIGURE_ITEM_SIZE);
        if (!s.ok()) {
            if (s.EndOfFile()) {
                break;
            }
            return s;
        }
        
/*
        if (item.Fit((const char*)item_buffer_, (int)CONFIGURE_ITEM_SIZE)) {
            *offset = off;
            return s; 
        }
*/

        new_item.Parse(item_buffer_, CONFIGURE_ITEM_SIZE);
        if ( strcmp(item.key.c_str(), new_item.key.c_str())== 0) {
            *offset = off;
            return s;
        }
 //       printf ("new item trace: --%s:%lld\n",new_item.key.c_str(), new_item.len);
        NextOffset(new_item, &off);
        if (OffsetFeb31 == off) break;
    }
    s.SetNotExist();
    return s;
}

int const CONFIGURE::GetFD(){
    return fd_;
}
/*
Status CONFIGURE::GetItemOffsetBlack(const Slice& key, Offset* offset) {
     Status s = GetItemOffset(key, offset);
     *offset += CONFIGURE_ITEM_SIZE; 
     return s;
};
*/
ITEM_MAP::iterator CONFIGURE::ITEM_MAP_FIND(ITEM_MAP& items_map, std::string& key) {
    ITEM_MAP::iterator it = items_map.begin();
    for ( ; it != items_map.end(); it++) {
        if (strcmp(key.c_str(), it->first.c_str())==0) break;
    }
    return it;
}

}; // namespace kvs

