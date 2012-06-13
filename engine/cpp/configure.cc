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

//Memory&Visual Only
Status CONFIGURE::Get(const Slice& key, std::string* property) {
    ITEM_MAP::iterator it = items_.find(key.data());
    if (it == items_.end()) {
        s.SetNotFound();
    } else {
        *property = items_[key.data()].value;
    }
    return s;
}

//Memory&Visual Only
Status CONFIGURE::Set(const Slice& key, const Slice& property) {
    Status s;
    ITEM_MAP::iterator it = items_.find(key.data());
    if (it == items_.end()) {
        s.SetNotFound();
    } else {
        ITEM item(key, property);
        items_[key.data()]=item;
    }
    return s;
}

Status CONFIGURE::Set(const ITEM& item) {
    Status s;
    items_[item.key.data()] = item;
    return s;
}

//malloc in here
Status CONFIGURE::NewItem(const Slice& key, const Offset len) {
    Status s;
    char* buf = malloc(len);
    ITEM item(key, len, buf, len);
    items_[key.data()] = item;
    return s;
} 

Status CONFIGURE::NewItem(const Slice& key) {
    Status s;
    ITEM item(key, OffsetNotExist, NULL, 0);
    items_[key.data()] = item;
    return s;
}

Status CONFIGURE::NewItem(const Slice& key, const Slice& value) {
   Status s;
   ITEM item(key, value);
   items_[key.data()] = item; 
   return s;
}

Status CONFIGURE::NewItem(const ITEM& item)
{
   Status s;
   items_[item.key.data()] = item;
   return s;
}

char* CONFIGURE::GetSpace(const Slice& key) {
    return items_[key.data()].buf;
}


//Memory Only
Offset CONFIGURE::GetLastOffset() {
    return last_offset;
}

Status CONFIGURE::Solid() {
    Status s;
    if (blank_) {
        std::list<ITEM> black_items;
        std::list<ITEM> last_items;
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
        Item item(item_buffer_, CONFIGURE_ITEM_SIZE);
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
Status CONFIGURE::AppendItem(const ITEM& item, Offset *offset) {
    Status s;
    
    item.serialize(item_buffer_, CONFIGURE_ITEM_SIZE);
    s = WriteFile(fd_, *offset, item_buffer_, CONFIGURE_ITEM_SIZE);
    if (!s.ok()) return s;
    if (black == item.type && OffsetFeb31 != item.len) {
        s = WriteFile(fd_, *offset + CONFIGURE_ITEM_SIZE,  item.buf, item.len);
        if (!s.ok()) return s;
        *offset += item.len;
    }
    
    //do it at last for rollback
    *offset += CONFIGURE_ITEM_SIZE;
    return s;
}

Status CONFIGURE::InjectItem(const ITEM& item) {
    Status s;
    Offset off;
    s = SearchItemOffset(item, &off);
    if (!s.ok()) return s;
    s = AppendItem(item, off);
    if (!s.ok()) return s;

    return s;
}

//go to head
void CONFIGURE::NextOffset(const ITEM& item, Offset *offset) {
    *offset += CONFIGURE_ITEM_SIZE;
    if (item.type == black)  *offset += item.len;
    if (OffsetFeb31 == item.len) *ffset = OffsetFeb31;
    return;
}

Status CONFIGURE::FetchLastOffset(Offset *off) {
    //Linux 返回文件最后的offset
    return GetFileLast(fd_, off);
}

Status CONFIGURE::FetchItemBuffer(const Offset off)
{
    Offset off = 0;
    Status s;
    memset(item_buffer, 0, sizeof(item_buffer));
    s = ReadFile(fd_, off, item_buffer, CONFIGURE_ITEM_SIZE);
    if (!s.ok()) return s;
    return s;
}


Status CONFIGURE::SearchItemOffset(const Item& item, Offset* offset) {
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
        
        if (item.Fit(item_buffer, CONFIGURE_ITEM_SIZE)) {
            *offset = off;
            return; 
        }
/*
        if (strncmp(key.data(), item_buffer, key.size()) == 0) {
            *offset = off;
            return s;
        }
*/
        NextOffset(item, &off);
        if (OffsetFeb31 == off) break;
    }
    s.SetNotExist();
    return s;
}

Status CONFIGURE::GetItemOffsetBlack(const Slice& key, Offset* offset) {
     Status s = GetItemOffset(key, offset);
     *offset += CONFIGURE_ITEM_SIZE; 
     return s;
};

}; // namespace kvs
