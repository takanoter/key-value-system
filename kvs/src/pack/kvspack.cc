#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kvspack.h"

namespace kvs {

typedef struct _ITEM_HEAD {
    int value_size;
    int name_size;
    int value_type;
} _ITEM_HEAD;


bool PACK::Init(char* buf, int buf_len) {
    buf_size_ = buf_len;
    buf_ = buf;
    send_length_ = (int*)(buf_);
    *(send_length_) = KVS_PACK_HEAD_SIZE;
    return true;
}

bool PACK::Load(char* buf, int buf_size) {
    buf_ = buf;
    buf_size_ = buf_size;
    send_length_ = (int*)(buf_);
    *(send_length_) = *((int*)(buf_));
    return true;
}

int PACK::Put(char* name, int name_len, int type, char* value, int value_len) 
{
    int ret = 0;
    char *p = NULL;
    _ITEM_HEAD item_head;
    
    item_head.value_size = value_len;
    item_head.name_size = name_len;
    item_head.value_type = type;

    p = buf_;
    p += *(send_length_);
    memcpy((void*)p, (const void*)&item_head, (size_t)sizeof(item_head));
    p += sizeof(item_head);
    memcpy(p, name, name_len);
    p += name_len;
    memcpy(p, value, value_len);

    *(send_length_) += sizeof(item_head) + name_len + value_len;
    return ret;
}

int PACK::Get(char* name, int name_len, int *type, char **value, int *value_len)
{
    int cur_offset = KVS_PACK_HEAD_SIZE;
    char *cur_name = NULL;
    _ITEM_HEAD *item_head = NULL;
    while (cur_offset < *(send_length_)) {
        item_head = (_ITEM_HEAD*) &(buf_[cur_offset]);
        cur_name = (char*) &(buf_[cur_offset + sizeof(_ITEM_HEAD)]);
        if (strncmp(cur_name, name, name_len) == 0) 
        {
            *type = item_head->value_type;
            *value = (char*)&(buf_[cur_offset + sizeof(_ITEM_HEAD) + item_head->name_size]);
            *value_len = item_head->value_size;
            return 0;
        }
        cur_offset += sizeof(_ITEM_HEAD) + item_head->name_size + item_head->value_size;
    }   
    return -1;
}

char* PACK::GetBuf()
{
    return (buf_);
}

int PACK::GetSendLength()
{
    return *(send_length_);
}

void PACK::Check()
{
    int i;
    printf("send_length:%d\n", *(send_length_));
    printf("buf_size:%d\n", buf_size_);
    printf("buf:");
    for (i = 0; i < *(send_length_); i++) {
        printf ("[%c]", buf_[i]);
    }
    printf ("\n\n");
    return;
}

}; //namespace kvs
