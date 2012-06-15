/*************************************************************************
    > File Name: engine.cc
    > Author: takanoter@gmail.com
*/
//*FIXME this is a hash+tree engine.

#include "kvs_status.h"
#include "configure.h"
#include "index.h"

namespace kvs {

INDEX::~INDEX() {
    if (hash_head_ != NULL) free(hash_head_);
}

Status INDEX::Load(CONFIGURE& conf, const int index_head_size) {
    Status s = Init(conf, index_head_size);
    if (!s.ok()) return s;

    for (int i=0; i<item_horizon_; i++) {
        char* item = GetItemFromBSTNode(i);
        if (NULL == item) continue;
        s = BSTInsert(true, item);
        if (!s.ok()) return s;
    }
    return s;
}

Status INDEX::Born(CONFIGURE& conf, const int index_head_size) {
    Status s = Init(conf, index_head_size);
    if (!s.ok()) return s;
    assert(item_horizon_==0);
    return s;
}

Status INDEX::Search(const Slice& key, Offset* off, Offset* len) {
    Status s;
    if (key_len_ != key.size()) {
        s.SetInvalidParam("key_length", key.size());
        return s;
    }
    char* item = FillItem(key);
    Offset node_id = BSTSearchId(item);
    if (OffsetFeb31 == node_id) {
        s.SetNotExist();
        return s;
    }
    *off = GetNodeOffset(node_id);
    *len = GetNodeLength(node_id);
    return s;
}

Status INDEX::Insert(const bool cover, const Slice& key, const Offset off, const Offset len) {
    Status s;
    if (key_len_ != key.size()) {
        s.SetInvalidParam("key_length", key.size());
        return s;
    }
    char* item = FillItem(key, off, len);
    s = BSTInsert(cover, item);
    if (!s.ok()) return s;
    return s;
}

Status INDEX::Del(const Slice& key) {
    Status s;
    if (key_len_ != key.size()) {
        s.SetInvalidParam("key_length", key.size());
        return s;
    }
    char* item = FillItem(key);
    s = BSTDelete(item);
    if (!s.ok()) return s;
    return s;
}


/********** private *******************************/

Offset INDEX::BSTSearchId(char* item)
{
    Offset ht_id = GetHTId(item);
    Offset node_id = hash_head_[ht_id];
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

Status INDEX::BSTInsert(const bool cover, char* item)
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

    Offset free_node_id = PopFreeNode();
    FillNewNode(free_node_id, item);
    *pre_node_ptr = free_node_id;

    return s;
}

Status INDEX::BSTDelete(char* item) {
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


Status INDEX::Init(CONFIGURE& conf, const int index_head_size) {
    Slice k;
    std::string p;
    char* buf = NULL;

    k.Set((const char*)"key_length");
    Status s = conf.Get(k, &p);
    if (!s.ok()) return s;
    key_len_ = atoi(p.c_str());
    assert((key_len_ == 8) || (key_len_ == 16));

    k.Set("index");
    item_ = conf.GetBuffer(k); 
    assert(item_ != NULL);
    item_num_ = INDEX_ITEM_NUM;
    k.Set("index_horizon");
    s = conf.Get(k, &p);
    if (!s.ok()) return s;
    item_horizon_ = atoi(p.c_str());

    hash_head_size_ = index_head_size;

    k.Set("index_free_slot");
    buf = conf.GetBuffer(k);
    assert(buf != NULL);
    k.Set("index_free_slot_horizon");
    s = conf.Get(k, &p);
    if (!s.ok()) return s;
    Offset free_slots_horizon = atoi(p.c_str());
    free_slots_.Fill(buf, INDEX_FREE_SLOT_FIX_SIZE, free_slots_horizon);

    rollback_item_ = NULL;
    last_operation_ = kDoNothing; 

    hash_head_ = (Offset*)malloc(index_head_size * sizeof(Offset));
    for (int i=0; i<hash_head_size_; i++) {
        hash_head_[i] = OffsetFeb31;
    }
         
    return s;
}

Offset INDEX::PopFreeNode() {
    Offset node;
    node = free_slots_.Pop();
    if (OffsetFeb31 == node) {
        if (item_horizon_ == item_num_) {
            return OffsetFeb31;
        }
        node = item_horizon_;
        item_horizon_++;
    }
    return node;
}

//放进去前，将item_[node_id]设置下，防止下次load时候再次加载
void INDEX::PushFreeNode(const Offset node_id) { 
    if (8 == key_len_) {
        INDEX_ITEM_8* item_8 = (INDEX_ITEM_8*)item_;
        item_8[node_id].key = OffsetFeb31;
    } else if (16 == key_len_) {
        INDEX_ITEM_16* item_16 = (INDEX_ITEM_16*)item_;
        item_16[node_id].key[0] = OffsetFeb31;
        item_16[node_id].key[1] = OffsetFeb31;
    } else {
        // do nothing;
    }
    free_slots_.Push(node_id);
}

char* INDEX::GetItemFromBSTNode(const Offset node_id) {
    char* item = NULL;
    if (8 == key_len_) {
        INDEX_ITEM_8* item_8 = (INDEX_ITEM_8*)item_;
        item = (char*)(&item_8[node_id]);
    } else if (16 == key_len_) {
        INDEX_ITEM_16* item_16 = (INDEX_ITEM_16*)item_;
        item = (char*)(&item_16[node_id]);
    } else {
        // do nothing;
    }
    return item;
}

char* INDEX::FillItem(const Slice& key) {
    if (8 == key_len_) {
        memcpy(&item_8_.key, key.data(), 8);
        item_8_.offset = OffsetFeb31;
        item_8_.length = OffsetFeb31;
        item_8_.right = item_8_.left = OffsetFeb31;
        return (char*)(&item_8_);
    } else if (16 == key_len_) {
        memcpy(&item_16_.key[0], key.data(), 16);
        item_16_.offset = OffsetFeb31;
        item_16_.length = OffsetFeb31;
        item_16_.right = item_16_.left = OffsetFeb31;
        return (char*)(&item_16_);
    };
    return NULL;
}

char* INDEX::FillItem(const Slice& key, const Offset offset, const Offset length) {
    if (8 == key_len_) {
        memcpy(&item_8_.key, key.data(), 8);
        item_8_.offset = offset;
        item_8_.length = length;
        item_8_.right = item_8_.left = OffsetFeb31;
        return (char*)(&item_8_);
    } else if (16 == key_len_) {
        memcpy(&item_16_.key[0], key.data(), 16);
        item_16_.offset = offset;
        item_16_.length = length;
        item_16_.right = item_16_.left = OffsetFeb31;
        return (char*)(&item_16_);
    };
    return NULL;
}

void INDEX::FillNewNode(const Offset node_id, char *item) {
    if (8 == key_len_) {
        INDEX_ITEM_8* item_8 = (INDEX_ITEM_8*)item_;
        INDEX_ITEM_8* item_p = (INDEX_ITEM_8*)item;
        
        item_8[node_id].key = item_p->key;
        item_8[node_id].offset = item_p->offset;
        item_8[node_id].length = item_p->length;
        item_8[node_id].right = item_p->right;
        item_8[node_id].left = item_p->left;
    } else if (16 == key_len_) {
        INDEX_ITEM_16* item_16 = (INDEX_ITEM_16*)item_;
        INDEX_ITEM_16* item_p = (INDEX_ITEM_16*)item;

        item_16[node_id].key[0] = item_p->key[0];
        item_16[node_id].key[1] = item_p->key[1];
        item_16[node_id].offset = item_p->offset;
        item_16[node_id].length = item_p->length;
        item_16[node_id].right = item_p->right;
        item_16[node_id].left = item_p->left;
    } else {
        // do nothing;
    }
    return ;
}

Offset INDEX::GetNodeOffset(const Offset node_id) {
    if (8 == key_len_) {
        INDEX_ITEM_8* item_8 = (INDEX_ITEM_8*)item_;
        return item_8[node_id].offset;
    } else if (16 == key_len_) {
        INDEX_ITEM_16* item_16 = (INDEX_ITEM_16*)item_;
        return item_16[node_id].offset;
    } else {
        // do nothing;
    }
    return OffsetFeb31;
}

Offset INDEX::GetNodeLength(const Offset node_id) {
    if (8 == key_len_) {
        INDEX_ITEM_8* item_8 = (INDEX_ITEM_8*)item_;
        return item_8[node_id].length;
    } else if (16 == key_len_) {
        INDEX_ITEM_16* item_16 = (INDEX_ITEM_16*)item_;
        return item_16[node_id].length;
    } else {
        // do nothing;
    }
    return OffsetFeb31;
}

Offset INDEX::GetHTId(char *item) {
    Offset key;
    if (8 == key_len_) {
        INDEX_ITEM_8* item_p = (INDEX_ITEM_8*)item;
        key = item_p->key;
    } else if (16 == key_len_) {
        INDEX_ITEM_16* item_p = (INDEX_ITEM_16*)item;
        key = item_p->key[0];
    } else {
        // do nothing;
    }
    return key % hash_head_size_;
}

int INDEX::BSTNodeCmp(char *item, const Offset node_id) {
    if (8 == key_len_) {
        Offset key;
        INDEX_ITEM_8* item_p = (INDEX_ITEM_8*)item;
        INDEX_ITEM_8* item_8 = (INDEX_ITEM_8*)item_;
        key = item_p->key;
        if (key == item_8[node_id].key) return 0;
        if (key < item_8[node_id].key) return -1;
        return 1;
    } else if (16 == key_len_) {
        Offset key[2];
        INDEX_ITEM_16* item_p = (INDEX_ITEM_16*)item;
        INDEX_ITEM_16* item_16 = (INDEX_ITEM_16*)item_;
        key[0] = item_p->key[0];
        key[1] = item_p->key[1];
        if (key[0] == item_16[node_id].key[0]) {
            if (key[1] == item_16[node_id].key[1]) return 0;
            if (key[1] < item_16[node_id].key[1]) return -1;
            return 1;
        }
        if (key[0] < item_16[node_id].key[0]) return -1;
        return 1;
    } else {
        // do nothing;
    }
    return 0;
}

Offset INDEX::GetBSTLeftNodeId(const Offset node_id) {
    if (8 == key_len_) {
        INDEX_ITEM_8* item_8 = (INDEX_ITEM_8*)item_;
        return item_8[node_id].left;
    } else if (16 == key_len_) {
        INDEX_ITEM_16* item_16 = (INDEX_ITEM_16*)item_;
        return item_16[node_id].left;
    } else {
        // do nothing;
    }
    return OffsetFeb31;
}

void INDEX::SetBSTLeftNodeId(const Offset node_id, const Offset v) {
    if (8 == key_len_) {
        INDEX_ITEM_8* item_8 = (INDEX_ITEM_8*)item_;
        item_8[node_id].left = v;
    } else if (16 == key_len_) {
        INDEX_ITEM_16* item_16 = (INDEX_ITEM_16*)item_;
        item_16[node_id].left = v;
    } else {
        // do nothing;
    }
    return ;
}

void INDEX::SetBSTRightNodeId(const Offset node_id, const Offset v) {
    if (8 == key_len_) {
        INDEX_ITEM_8* item_8 = (INDEX_ITEM_8*)item_;
        item_8[node_id].right= v;
    } else if (16 == key_len_) {
        INDEX_ITEM_16* item_16 = (INDEX_ITEM_16*)item_;
        item_16[node_id].right= v;
    } else {
        // do nothing;
    }
    return ;
}

Offset INDEX::GetBSTRightNodeId(const Offset node_id) {
    if (8 == key_len_) {
        INDEX_ITEM_8* item_8 = (INDEX_ITEM_8*)item_;
        return item_8[node_id].right;
    } else if (16 == key_len_) {
        INDEX_ITEM_16* item_16 = (INDEX_ITEM_16*)item_;
        return item_16[node_id].right;
    } else {
        // do nothing;
    }
    return OffsetFeb31;
}

Offset* INDEX::GetBSTLeftNodePtr(const Offset node_id) {
    if (8 == key_len_) {
        INDEX_ITEM_8* item_8 = (INDEX_ITEM_8*)item_;
        return &(item_8[node_id].left);
    } else if (16 == key_len_) {
        INDEX_ITEM_16* item_16 = (INDEX_ITEM_16*)item_;
        return &(item_16[node_id].left);
    } else {
        // do nothing;
    }
    return NULL;
}

Offset* INDEX::GetBSTRightNodePtr(const Offset node_id) {
    if (8 == key_len_) {
        INDEX_ITEM_8* item_8 = (INDEX_ITEM_8*)item_;
        return &(item_8[node_id].right);
    } else if (16 == key_len_) {
        INDEX_ITEM_16* item_16 = (INDEX_ITEM_16*)item_;
        return &(item_16[node_id].right);
    } else {
        // do nothing;
    }
    return NULL;
}

};  // namespace kvs 
