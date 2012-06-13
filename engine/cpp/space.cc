/*************************************************************************
    > File Name: space.h
    > Author: takanoter@gmail.com
*/


#ifndef KVS_ENGINE_INCLUDE_SPACE_H_
#define KVS_ENGINE_INCLUDE_SPACE_H_

#include <stdio.h>
#include "kvs_status.h"
#include "kvs_slice.h"

namespace kvs {

Status SPACE::Load(Configure& data_conf) {
    Status s = data_conf.FetchLastOffset(&last_offset_);
    if (!s.ok()) return s;
    fd_ = data_conf.GetFD();
    conf_ = data_conf;

    std::string key_len;
    s = data_conf.Get("key_length", &key_len);
    int key_length = key_len.toInt32();
    item_head_length_ = key_length + sizeof(Offset)*2;
    return s;
}

Status SPACE::Born(Configure& data_conf) {
    Status s = data_conf.FetchLastOffset(&last_offset_);
    if (!s.ok()) return s;
    fd_ = data_conf.GetFD();
    conf_ = data_conf;

    std::string key_len;
    s = data_conf.Get("key_length", &key_len);
    int key_length = key_len.toInt32();
    item_head_length_ = key_length + sizeof(Offset)*2;
    return s;
}

Status SPACE::Read(const Offset offset, const Offset length, std::string* value) {
    Status s = ReadFile(fd_, offset, buffer_, length); 
    if (!s.ok()) return s;
    *value(buffer[item_head_length], length - item_head_length);
    return s;
}

Status SPACE::Write(const Offset offset, const Offset id, const Slice& key, const Slice& value, 
                    const bool sync) {
//FIXME:memcpy...
    Offset len = 0;
    memcpy(buffer_[len], &id, sizeof(id));
    len += sizeof(Offset);
    memcpy(buffer_[len], key.data(), key.size());
    len += key.size();
    memcpy(buffer_[len], value.size(), sizeof(Offset));
    len += sizeof(Offset);
    memcpy(buffer_[len], value.data(), value.size());
    len += value.size();
    s = WriteFile(fd_, offset, buffer_, len); 
    SetUse(len);
}

Offset SPACE::GetSpace() {
    return last_offset_t;
}

Status SPACE::SetUse(Offset len)  {
   last_offset_t += len;
   //data_conf不设置
   //conf_.Set("data", last_offset_t);
}

}; // class SPACE

}; // namespace kvs
#endif // KVS_ENGINE_INCLUDE_SPACE_H_
