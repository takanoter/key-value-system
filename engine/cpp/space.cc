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

Offset SPACE::GetSpace() {
    return last_offset_;
}

Status SPACE::SetUse(Offset len)  {
   last_offset_ += len;
   //data_conf不设置
   //conf_.Set("data", last_offset_t);
}

}; // namespace kvs
