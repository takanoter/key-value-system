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

class FREE_SLOT {
  public:
    FREE_SLOT() 
        : size_(DEFAULT_POOL_SIZE),
        : horizon_(0) {
	slots_ = new (sizeof(Offset)* size_);
    };

    ~FREE_SLOT() {
	delete slots_;
    };
   
    Offset Size();
    Offset Horizon();
    Offset TotalSize();


  private:
    Offset [] slots_;
    Offset size_;
    Offset horizon_;


}; // class FREE_SLOT
    

class INDEX {
  public:
    INDEX() {};
    ~INDEX();

    Status Search(const Slice& key, Offset off, Offset len);
    Status Insert(const bool cover, const Slice& key, Offset off, Offset id_);
    Status Del(const Slice& key);

    Status Load(const Configure& conf_, const int index_head_size);
    Status Born(const int index_head_size);
    Status Save(const Configure& conf_);

    Status Backward(); //FIXME only can back one step, may be we can use id_;
  private:
    int key_len;
    int item_length;

    void *item;
    Offset [] hash_head;
    int item_size;
    int hash_head_size;

    FREE_SLOT free_slots;

    void *rollback_item;
    OPERATION last_operation;

}; // class INDEX


}; // namespace kvs
#endif // KVS_ENGINE_INCLUDE_INDEX_H_
