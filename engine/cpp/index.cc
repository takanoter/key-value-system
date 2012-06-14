/*************************************************************************
    > File Name: engine.cc
    > Author: takanoter@gmail.com
*/
//*FIXME this is a hash+tree engine.

#include "kvs_engine.h"
#include "engine.h"

namespace kvs {

INDEX::INDEX() {
    //init Writer or sth else memory
}

INDEX::~INDEX() {
}

Status INDEX::Load(const Configure& conf, const int index_head_size) {
    s = Init(conf, index_head_size);
    if (!s.ok()) return s;
    for (i=0; i<item_horizon_; i++) {
        if (8 == key_len) 


    }
    return s;
}

Status INDEX::Init(const Configure& conf, const int index_head_size) {
    Slice k;
    std::string p;
    char* buf = NULL;

    k.data("key_length");
    s = conf.Get(key, &p);
    if (!s.ok()) return s;
    key_len_ = atoi(p.c_str());
    assert((key_len == 8) || (key_len == 16));

    k.data("index");
    item_ = conf.GetBuffer(key); 
    assert(item_ != NULL);
    item_num_ = INDEX_ITEM_NUM;
    k.data("index_horizon");
    s = conf.Get(key, &p);
    if (!s.ok()) return s;
    item_horizon_ = atoi(p.c_str());

    hash_head_ = malloc(index_head_size * sizeof(Offset));
    hash_head_size_ = index_head_size;

    k.data("index_free_slot");
    buf = conf.GetBuffer(key);
    assert(buf != NULL);
    k.data("index_free_slot_horizon");
    s = conf.Get(key, &p);
    if (!s.ok()) return s;
    Offset free_slots_horizon = atoi(p.c_str());
    free_slots_.Fill(buf, INDEX_FREE_SLOT_FIX_SIZE, free_slots_horizon);

    rollback_item = NULL;
    last_operation_ = OP_NOTHING; 
    return s;
}

Status INDEX::Born(const Configure& conf, const int index_head_size) {
    s = Init(conf, index_head_size);
    if (!s.ok()) return s;
    assert(item_horizon_==0);
    return s;
}

Status INDEX::Search(const Slice& key, Offset* off, Offset* len) {


}

Status INDEX::Insert(const bool cover, const Slice& key, Offset off, Offset id_) {


}

Status Del(const Slice& key) {


}


};  // namespace kvs 

