/*************************************************************************
    > File Name: index.h
    > Author: takanoter@gmail.com
*/
#ifndef KVS_ENGINE_INCLUDE_INDEX_H_
#define KVS_ENGINE_INCLUDE_INDEX_H_

#include <stdio.h>
#include "kvs_status.h"
#include "kvs_options.h"
#include "kvs_slice.h"

namespace kvs {

/* {key,length, offset, tree} */
/*FIXME: template*/
const int INDEX_ITEM_8_SIZE = 8/*key*/+4/*length*/+8/*offset*/+8*2/*tree*/;
const int INDEX_ITEM_16_SIZE = 16/*key*/+4/*length*/+8/*offset*/+8*2/*tree*/;
const int INDEX_ITEM_NUM = 1024*1024*10;
const Offset INDEX_FREE_SLOT_FIX_NUM = 1024*1024;
const Offset INDEX_FREE_SLOT_FIX_SIZE = INDEX_FREE_SLOT_FIX_NUM * sizeof(Offset);
const Offset INDEX_FIX_8_SIZE = INDEX_ITEM_NUM * INDEX_ITEM_8_SIZE;
const Offset INDEX_FIX_16_SIZE = INDEX_ITEM_NUM * INDEX_ITEM_16_SIZE;

struct INDEX_ITEM_8 {
    Offset key;
    int length;
    Offset offset;
    Offset left,right;
};

struct INDEX_ITEM_16 {
    Offset key1,key2;
    int length;
    Offset offset;
    Offset left,right;
};
/***
ItemCmp
ItemFill
ItemMove
*/

class FREE_SLOT {
  public:
    FREE_SLOT() {
    };

    ~FREE_SLOT() {
    };
   
    void Fill(const void* buf, const Offset size, const Offset horizon) {
        slots_ = buf;
        size_ = size;
        horizon_ = horizon;
    }

    Offset Size();
    Offset Horizon();
    Offset TotalSize();

  public:
    void* slots_;

  private:
    Offset size_;
    Offset horizon_;

}; // class FREE_SLOT
    

class INDEX {
  public:
    INDEX() {};
    ~INDEX();

    Status Search(const Slice& key, Offset* off, Offset* len);
    Status Insert(const bool cover, const Slice& key, Offset off, Offset id_);
    Status Del(const Slice& key);

    Status Load(const Configure& conf_, const int index_head_size);
    Status Born(const Configure& conf_, const int index_head_size);

    Status Backward(); //FIXME only can back one step, may be we can use id_;

  private:
    int key_len_;

    void* item_; //real index items
    Offset* hash_head_; //hash head
    int item_num_; //use default: INDEX_ITEM_NUM
    int hash_head_size_;
    Offset item_horizon_;

    FREE_SLOT free_slots_; //free slot

    //Rollback
    void *rollback_item_;
    OPERATION last_operation_;

  private:
    Status Init(const Configure& conf_, const int index_head_size);

}; // class INDEX


}; // namespace kvs
#endif // KVS_ENGINE_INCLUDE_INDEX_H_
