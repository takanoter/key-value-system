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

    hash_head_ = (Offset*)malloc(index_head_size * sizeof(Offset));
    for (int i=0; i<index_head_size_; i++) {
        hash_head_[i] = OffsetFeb31;
    }
         
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

Offset BSTSearchId(const void* item)
{
    Offset ht_id = GetHTId(item);
    Offset node_id = hash_head_[ht_id};

    while (node_id != OffsetFeb31)
    {	
        int cmp = BSTNodeCmp(item, node_id); 
        if (cmp == 0) {
            return node_id;
        } else if (cmp < 0) {
            node_id = GetBSTLeftNodeId(node_id);
        } else {
            node_id = GetBSTRightNodeId(node_id);
        }
    }

    return OffsetFeb31;
}

Status BSTInsert(const bool cover, const void* item)
{
    Status s;
    Offset ht_id = GetHTId(item);
    Offset node_id = hash_head_[ht_id];
    Offset* pre_node_ptr = &hash_head_[ht_id];

    while (node_id != OffsetFeb31) {
        int cmp = BSTNodeCmp(item, node_id); 
        if (cmp == 0) {
            if (!cover) {
                s.SetExist();
                return s;
            }
            break;
        } else if (cmp < 0) {
            pre_node_ptr = GetBSTLeftNodePtr(node_id);
            node_id = GetBSTLeftNodeId(node_id);
        } else {
            pre_node_ptr = GetBSTRightNodePtr(node_id);
            node_id = GetBSTRightNodeId(node_id);
        }
    } 

    free_node_id = PopFreeNode();
    FillNewNode(free_node_id, item);
    *pre_node_ptr = free_node_id;
    PushNodeToIndex(item, 

    return s;
}

Status BSTDelete(const void* item) {
    Status s;
    Offset ht_id = GetHTId(item);
    Offset node_id = hash_head_[ht_id];
    Offset* pre_node_ptr = &hash_head_[ht_id];

    while (true)
    {
        if (OffsetFeb31 == node_id) {
            s.SetNotExist();
            return s;
        }
        int cmp = BSTNodeCmp(item, node_id); 
        if (cmp == 0) {
            break;
        } else if (cmp < 0) {
            pre_node_ptr = GetBSTLeftNodePtr(node_id);
            node_id = GetBSTLeftNodeId(node_id);
        } else {
            pre_node_ptr = GetBSTRightNodePtr(node_id);
            node_id = GetBSTRightNodeId(node_id);
        }
    } // end of while

    if (GetBSTLeftNodeId(node_id) == OffsetFeb31 
        && GetBSTRightNodeId(node_id) == OffsetFeb31) {
        *pre_node_ptr = OffsetFeb31;
    } else if (GetBSTLeftNodeId(node_id) != OffsetFeb31
               && GetBSTRightNodeId(node_id) != OffsetFeb31) {
        Offset* nearest_pre_node_ptr = GetBSTRightNodePtr(node_id);
        Offset nearest_id = GetBSTRightNodeId(node_id);
        while (GetBSTLeftNodeId(node_id) != OffsetFeb31) {
            nearest_pre_node_ptr = GetBSTLeftNodePtr(nearest_id);
            nearest_id = GetBSTLeftNodeId(nearest_id);
        }
        *nearest_pre_node_ptr = GetBSTRightNodeId(nearest_id);
        SetBSTLeftNodeId(nearest_id, GetBSTLeftNodeId(node_id));
        SetBSTRightNodeId(nearest_id, GetBSTRightNodeId(node_id));
        *pre_node_ptr = nearest_id;
    } else if (GetBSTLeftNodeId(node_id) != OffsetFeb31) {
        *pre_node_ptr = GetBSTLeftNodeId(node_id);
    } else {
        *pre_node_ptr = GetBSTRightNodeId(node_id);
    }

    PushFreeNode(node_id); //放进去前，将item_[node_id]设置下，防止下次load时候再次加载
    return s;
}


static unsigned long long _get_free_idx_node(INDEX *idx)
{
    unsigned long long ret = 0;
    /*TO DO if xx < 0 */
    if (0 == idx->free_node_count) {
        ret = idx->node_pool_horizon;
        ++(idx->node_pool_horizon);
    } else {
        ret = idx->free_node_pool[idx->free_node_count];
        --(idx->free_node_count);
    }
	return ret;
}

static void _put_free_idx_node(INDEX* idx, unsigned long long node_id)
{
    /*TO DO : some check */
	idx->free_node_pool[++(idx->free_node_count)] = node_id;
}

