#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "kvs_api.h"

int kv_open(KVS **kvs, const char *dbfilename, KVS_OPT *option)
{
	KVS* kvs_desp = NULL;
	int ret = err_success;
	if (NULL == kvs || NULL == dbfilename || NULL == option) {
		ret = err_input_param_null;
		goto OUT;
	}
	
	kvs_desp = (KVS* ) malloc(sizeof(KVS));
	if (NULL == kvs_desp) {
		ret = err_malloc_failed;
		goto OUT;
	}
	
	kvs_desp->option = *option;

    /*FIXME:snprintf return value*/
	snprintf(kvs_desp->idx_name, sizeof(kvs_desp->idx_name), "%s.idx", dbfile);
	snprintf(kvs_desp->data_name, sizeof(kvs_desp->data_name), "%s.data", dbfile);
	 	
/*
	kvs_desp->idx_db = fopen(kvs_desp->idx_name, "w");
	kvs_desp->data_db = fopen(kvs_desp->data_name, "w");
	if (NULL == kvs_desp->idx_db || NULL == kvs_desp->data_db) {
		ret = err_open_file_failed;
		goto OUT;
	}
*/
    
    /* init index information*/
    kvs_desp->idx = NULL;
    if (CREATE == option->open_mode) {
        kvs_desp->idx = idx_create(kvs_desp->idx_name, option->key_num);
    } else if (LOAD == option->open_mode) {
        kvs_desp->idx = idx_load(kvs_desp->idx_name);
    }
    if (NULL == kvs_desp->idx) {
        printf ("index init failed.");
        ret = err_init_index;
        goto OUT;
    }

    /* init data information*/
    kvs_desp->data = NULL;
    if (CREATE == option->open_mode) {
        kvs_desp->data = data_create(kvs_desp->data_name);
    } else if (LOAD == option->open_mode) {
        kvs_desp->data = data_load(kvs_desp->data_name);
    }
    if (NULL == kvs_desp->data) {
        printf ("data init failed.");
        ret = err_init_data;
        goto OUT;
    }
	
	*kvs = kvs_desp;

	return err_success;

OUT:
    if (NULL != kvs_desp->data) {
        data_close(kvs_desp->data);
        kvs_desp->data = NULL;
    }

    if (NULL != kvs_desp->idx) {
        idx_close(kvs_desp->idx);
        kvs_desp->idx = NULL;
    }

	if (NULL != kvs_desp) {
		free(kvs_desp);
		kvs_desp = NULL;
	}
	return ret;
}

int kv_close(KVS **kvs)
{

	return 0;
}


int kv_put(KVS *kvs, void *key, int key_len, void *value, int value_len)
{
    int ret = err_success;
    unsigned long long timestamp = _kv_consequence_id();
   
    IDX_NODE idx_node;
    idx_node.key_sign = create_key_sign(key, key_len); 
    idx_node.value_offset = data_get_location(kvs->data, key_len+value_len);

	ret = idx_insert(kvs->idx, &idx_node, timestamp);
    if (ret != err_success) {
        return -1;
    }

    KV_PAIR kv_pair = kv_pair_create(timestamp, key, key_len, value, value_len);
    
    ret = data_insert(kvs->data, idx_node.value_offset, &kv_pair);
    if (ret != err_success) {
        return -1;
    }

	return ret;
}

int kv_get(KVS *kvs, KV_PAIR *kv_pair) 
{
    int ret = err_success;
    
    IDX_NODE idx_node;
    idx_node.key_sign = create_key_sign(kv_pair->key, kv_pair->key_len);

	ret = idx_search(kvs->idx, &idx_node); 
    if (ret != err_success) {
		return -1;
	}

    ret = data_get(kvs->data, idx_node.value_offset, kv_pair); 
    if (ret != err_success) {
        return -1;
    }

	return 0;
}

int kv_delete(KVS *kvs, const char* key, int key_size)
{
    int ret = err_success;
    unsigned long long timestamp = _kv_consequence_id();
   
    IDX_NODE idx_node;
    idx_node.key_sign = create_key_sign(key, key_len); 
    
    ret = idx_delete(kvs->idx, &idx_node, timestamp);
    if (err_success != ret) {
        return -1;
    }

    
    ret = data_delete(kvs->data, timestamp);
    if (err_success != ret) {
        return ret;
    }

	return 0;
}
