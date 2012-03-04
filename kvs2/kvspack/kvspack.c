#include <stdio.h>
#include <stdlib.h>
#include "kvspack.h"

typedef struct _ITEM_HEAD {
    int value_size;
    int name_size;
    int value_type;
} _ITEM_HEAD;

const int KVS_PACK_HEAD_SIZE = sizeof(int);

KVS_PACK* kvsp_create(void* buf, int buf_len)
{
    KVS_PACK* pack = NULL;
    pack = (KVS_PACK*) malloc(sizeof(KVS_PACK));
    if (NULL == pack) {
        goto OUT;
    }
    pack->buf_size = buf_len;
    pack->buf = buf;
    pack->send_length = (int*)(pack->buf);
    *(pack->send_length) = KVS_PACK_HEAD_SIZE;

    return pack;

OUT:
    if (NULL != pack) {
        free(pack);
        pack = NULL;
    }
    return NULL;
}

KVS_PACK* kvsp_load(void* buf, int buf_size)
{
    KVS_PACK* pack = NULL;
    pack = (KVS_PACK*) malloc(sizeof(KVS_PACK));
    if (NULL == pack) {
        goto OUT;
    }
    pack->buf = buf;
    pack->buf_size = buf_size;
    pack->send_length = (int*)(pack->buf);
    *(pack->send_length) = *((int*)(pack->buf));
    return pack;

OUT:
    if (NULL != pack) {
        free(pack);
        pack = NULL;
    }
    return NULL;
}

int kvsp_destroy(KVS_PACK* pack)
{
    if (pack == NULL) {
        return -1;
    }
    free(pack);
    return 0;
}

int kvsp_put(KVS_PACK* pack, void* name, int name_len, int type, void* value, int value_len)
{
    int ret = 0;
    char *p = NULL;
    _ITEM_HEAD item_head;
    
    item_head.value_size = value_len;
    item_head.name_size = name_len;
    item_head.value_type = type;

    p = pack->buf;
    p += *(pack->send_length);
    memcpy((void*)p, (const void*)&item_head, (size_t)sizeof(item_head));
    p += sizeof(item_head);
    memcpy(p, name, name_len);
    p += name_len;
    memcpy(p, value, value_len);

    *(pack->send_length) += sizeof(item_head) + name_len + value_len;
    return ret;
}

int kvsp_get(KVS_PACK* pack, const void* name, int name_len, int* type, void** value, int *value_len)
{
    int cur_offset = KVS_PACK_HEAD_SIZE;
    char *cur_name = NULL;
    _ITEM_HEAD *item_head = NULL;
    while (cur_offset < *(pack->send_length)) {
        item_head = (_ITEM_HEAD*) &(pack->buf[cur_offset]);
        cur_name = (char*) &(pack->buf[cur_offset + sizeof(_ITEM_HEAD)]);
        if (strncmp(cur_name, name, name_len) == 0) 
        {
            *type = item_head->value_type;
            *value = (char*)&(pack->buf[cur_offset + sizeof(_ITEM_HEAD) + item_head->name_size]);
            *value_len = item_head->value_size;
            return 0;
        }
        cur_offset += sizeof(_ITEM_HEAD) + item_head->name_size + item_head->value_size;
    }   
    return -1;
}

void* kvsp_getbuf(KVS_PACK* pack)
{
    if (NULL == pack) {
        return NULL;
    }
    return (void*)(pack->buf);
}

int kvsp_get_sendlen(KVS_PACK* pack)
{
    if (NULL == pack) {
        return -1;
    }
    return *(pack->send_length);
}

void kvsp_check(KVS_PACK *pack)
{
    int i;
    printf("send_length:%d\n", *(pack->send_length));
    printf("buf_size:%d\n", pack->buf_size);
    printf("buf:");
    for (i = 0; i < *(pack->send_length); i++) {
        printf ("[%c]", pack->buf[i]);
    }
    printf ("\n\n");
    return;
}
