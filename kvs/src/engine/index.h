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
#include "configure.h"

namespace kvs {

/* {key,length, offset, tree} */
/*FIXME: template*/
const int INDEX_ITEM_8_SIZE = 8/*key*/+4/*length*/+8/*offset*/+8*2/*tree*/;
const int INDEX_ITEM_16_SIZE = 16/*key*/+4/*length*/+8/*offset*/+8*2/*tree*/;
const int INDEX_ITEM_NUM = 1024; /*1024*1024*10*/
const Offset INDEX_FREE_SLOT_FIX_NUM = 1024; /*1024*1024*/
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
    Offset key[2];
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
   
    void Fill(char* buf, const Offset size, const Offset horizon);
    Offset Pop() ;
    void Push(const Offset id);
    Offset Horizon() ;

  public:
    Offset* slots_;

  private:
    Offset size_;
    Offset horizon_;

}; // class FREE_SLOT
    
enum OperationCode {
    kDoNothing = 0,
    kCoverInsert = 1,
    kUncoverInsert = 2,
    kDelete = 3
};

class INDEX {
  public:
    INDEX() {

    };
    ~INDEX();

    Status Search(const Slice& key, Offset* off, Offset* len);
    Status Insert(const bool cover, const Slice& key, const Offset off, const Offset len);
    Status Del(const Slice& key);

    Status Load(CONFIGURE& conf, const int index_head_size);
    Status Born(CONFIGURE& conf, const int index_head_size);

    //Status Backward(); //FIXME only can back one step, may be we can use id_;
    Offset GetIndexFreeSlotHorizon();
    Offset GetIndexHorizon();

  public:
    int hash_head_size_;

  private:
    int key_len_;

    char* item_; //real index items
    Offset* hash_head_; //hash head
    int item_num_; //use default: INDEX_ITEM_NUM
 
    Offset item_horizon_;

    FREE_SLOT free_slots_; //free slot

    //Rollback
    char* rollback_item_;
    OperationCode last_operation_;

    //buffer
    INDEX_ITEM_8 item_8_;
    INDEX_ITEM_16 item_16_;

  private:
    Status Init(CONFIGURE& conf, const int index_head_size);

    Offset BSTSearchId(char* item);
    Status BSTInsert(const bool cover, char* item);
    Status BSTDelete(char* item);

    Offset* GetBSTRightNodePtr(const Offset node_id);
    Offset* GetBSTLeftNodePtr(const Offset node_id);
    Offset GetBSTRightNodeId(const Offset node_id);
    Offset GetBSTLeftNodeId(const Offset node_id);
    void SetBSTRightNodeId(const Offset node_id, const Offset v);
    void SetBSTLeftNodeId(const Offset node_id, const Offset v);
    int BSTNodeCmp(char *item, const Offset node_id);

    Offset GetHTId(char *item);
    Offset GetNodeLength(const Offset node_id);
    Offset GetNodeOffset(const Offset node_id);

    char* FillItem(const Slice& key);
    char* FillItem(const Slice& key, const Offset offset, const Offset length);
    void FillNewNode(const Offset node_id, char *item);
    char* GetItemFromBSTNode(const Offset node);

    void PushFreeNode(const Offset node_id);
    Offset PopFreeNode();

}; // class INDEX


}; // namespace kvs
#endif // KVS_ENGINE_INCLUDE_INDEX_H_
