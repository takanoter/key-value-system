/*************************************************************************
    > File Name: space.cc
    > Author: takanoter@gmail.com
*/
#include <stdio.h>
#include "kvs_status.h"
#include "kvs_slice.h"
#include "space.h"

namespace kvs {

Status SPACE::Load(CONFIGURE& data_conf) {
    Status s = data_conf.FetchLastOffset(&last_offset_);
    if (!s.ok()) return s;
    fd_ = data_conf.GetFD();

    std::string key_len;
    s = data_conf.Get("key_length", &key_len);
    int key_length = atoi(key_len.c_str());
    item_head_length_ = key_length + sizeof(Offset)*2;
    return s;
}

Status SPACE::Born(CONFIGURE& data_conf) {
    Status s = data_conf.FetchLastOffset(&last_offset_);
    if (!s.ok()) return s;
    fd_ = data_conf.GetFD();

    std::string key_len;
    s = data_conf.Get("key_length", &key_len);
    int key_length = atoi(key_len.c_str());
    item_head_length_ = key_length + sizeof(Offset)*2;
    return s;
}

Status SPACE::Read(const Offset offset, const Offset length, std::string* value) {
    Status s = ReadFile(fd_, offset, buffer_, length); 
    if (!s.ok()) return s;
    /*FIXME copy?*/
    value->assign(&buffer_[item_head_length_], length - item_head_length_);
    return s;
}

Status SPACE::Write(const Offset offset, const Offset id, const Slice& key, const Slice& value, 
                    const bool sync) {
//FIXME:memcpy...
    Offset len = 0;
    memcpy(&buffer_[len], &id, sizeof(id));
    len += sizeof(Offset);
    memcpy(&buffer_[len], key.data(), key.size());
    len += key.size();

    Offset v_size = value.size(); 
    memcpy(&buffer_[len], &v_size, sizeof(Offset));
    len += sizeof(Offset);

    memcpy(&buffer_[len], value.data(), value.size());
    len += value.size();
    Status s = WriteFile(fd_, offset, buffer_, len); 
    SetUse(len);
    return s;
}

Offset SPACE::CalLength(const Slice& key, const Slice& value) {
    return sizeof(Offset)/*id*/ + key.size() + sizeof(Offset)/*value.size()*/ + value.size();
}

Offset SPACE::CalHeadLength(const Offset key_length) {
    return sizeof(Offset)/*id*/ + key_length + sizeof(Offset)/*value.size()*/;
}

Offset SPACE::CalLength(const Offset key_length, const Offset value_size) {
    return sizeof(Offset)/*id*/ + key_length + sizeof(Offset)/*value.size()*/ + value_size;
}

/*key,value use std::string not slice.*/
Status SPACE::Read(const Offset base_off, const Offset key_len, std::string *key, std::string* value, Offset* total_len) {
    assert((key_len==8) || (key_len==16));
    std::string v;
    Offset head_len = CalHeadLength(key_len);
    Status s = ReadFile(fd_, base_off, buffer_, head_len);
    if (!s.ok()) return s;

    Offset value_size;
    if (8 == key_len) {
        SPACE_HEAD_8* space_head_8 = (SPACE_HEAD_8*)buffer_;
        key->assign((char*)&(space_head_8->key), 8);
        value_size = space_head_8->value_len;
    } else if (16 == key_len) {
        SPACE_HEAD_16* space_head_16 = (SPACE_HEAD_16*)buffer_;
        key->assign((char*)&(space_head_16->key), 16);
        value_size = space_head_16->value_len;
    }

    *total_len = CalLength(key_len, value_size);
    s = Read(base_off, *total_len, value);
    if (!s.ok()) return s;

    return s;
}

Offset SPACE::GetSpace() {
    return last_offset_;
}

Status SPACE::SetUse(Offset len)  {
   last_offset_ += len;
   //data_conf不设置
   //conf_.Set("data", last_offset_t);
}

}; // namespace kvs
