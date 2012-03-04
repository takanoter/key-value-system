#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include "type.h"
#include "log.h"
#include "index.h"
#include "layout.h"

#define	HT_TABLE_MOD		1
#define	HT_TABLE_SIZE		131072
#define	IDX_NODE_NULL		30000000
#define IDX_NODE_INCREASE	2049

struct IDX_NODE
{
	HASH		hash_2, hash_3;
	PTR_DISK	disk_loc;
	int		value_len;
	PTR_KW		left_id, right_id;
};

static	FILE*			log_file=NULL;	
static	PTR_KW*			ht_table=NULL;  
static	struct IDX_NODE* 	idx_nodes=NULL;
static	PTR_KW*			idx_nodes_horizon=NULL;
static	PTR_KW*			free_idx_nodes=NULL;
static	PTR_KW*			free_idx_nodes_horizon=NULL;

static	int			_is_hash_same(int node_id, HASH hash_2, HASH hash_3);
static	PTR_KW			_get_free_idx_node();
static	void			_put_free_idx_node(PTR_KW id);
	HASH			hash_func_1(const char* key, int key_len);
	HASH			hash_func_2(const char* key, int key_len);
	HASH			hash_func_3(const char* key, int key_len);

STATE idx_init(INIT_TYPE init_type, const char* image, const char* index_log_file)
{
	int i;
	assert(image);
	
	if (index_log_file) {
		log_file=fopen(index_log_file, "w");
		if (log_file==NULL) {
			printf ("[%s:%d] %s: LOG_FILE:%s open failed. File may be exist, or try using SUDO.\n", __FILE__, __LINE__, strerror(errno), index_log_file);
			return IDX_NG_INIT_LOG;
		}
	} else {
		printf ("[%s:%d]warning: not specify \"index_log_file\".\n", __FILE__, __LINE__);
	}
		
	/* init the IndexImage Layout 
	 */
	ht_table=(PTR_KW*)(image+IMAGE_HT_TABLE);
	idx_nodes=(struct IDX_NODE*)(image+IMAGE_IDX_NODES);
	idx_nodes_horizon=(PTR_KW*)(image+IMAGE_IDX_NODES_HORIZON);
	free_idx_nodes=(PTR_KW*)(image+IMAGE_FREE_IDX_NODES);
	free_idx_nodes_horizon=(PTR_KW*)(image+IMAGE_FREE_IDX_NODES_HORIZON);	

	if (init_type==INIT_TYPE_CREATE) {
		for (i=0; i<HT_TABLE_SIZE; i++) {
            		ht_table[i]=IDX_NODE_NULL;
		}
		*idx_nodes_horizon=0;
		*free_idx_nodes_horizon=0;
	}
	log_err(__FILE__, __LINE__, log_file, "Index init type:%s", init_type==INIT_TYPE_LOAD?"load":"create");
	return IDX_SUCCESS;
}

void idx_exit()
{
	if (log_file) {
		fprintf (log_file, "EXIT\n");
		fclose(log_file);
	} else {
		printf ("index exit.\n");
	}
	log_file=NULL;
}

/*
 * return disk_location, value_len
 */
STATE idx_search(const char* key, int key_len, PTR_DISK* disk_location, int* value_len)
{
	int	ht_table_id, node_id, cmp;
	HASH	cur_hash_2, cur_hash_3;

	cur_hash_2 = hash_func_2(key, key_len);
	cur_hash_3 = hash_func_3(key, key_len);

	ht_table_id = hash_func_1(key, key_len) % HT_TABLE_MOD;  /* FIXME: hash_func_1() > 0  ?? */
	node_id = ht_table[ht_table_id];
	
	while (node_id != IDX_NODE_NULL)
	{
		cmp = _is_hash_same(node_id, cur_hash_2, cur_hash_3);
		if (cmp == 0) {
			*disk_location = idx_nodes[node_id].disk_loc;
			*value_len = idx_nodes[node_id].value_len;		
			return IDX_SUCCESS;
		} else if (cmp < 0) {
			node_id = idx_nodes[node_id].left_id;
		} else {
			node_id = idx_nodes[node_id].right_id;
		}
	}		
	return IDX_NG_NOT_EXIST;	
}	

STATE idx_insert(const char* key, int key_len, PTR_DISK disk_location, int value_len)
{
	int ht_table_id, node_id, new_node_id, cmp;
	HASH cur_hash_2, cur_hash_3;
	PTR_KW* pre_ptr;
	
	cur_hash_2 = hash_func_2(key, key_len);
	cur_hash_3 = hash_func_3(key, key_len);
	
	ht_table_id = hash_func_1(key, key_len) % HT_TABLE_MOD; /* FIXME: >0 ?? */
	pre_ptr = &ht_table[ht_table_id];
	node_id = ht_table[ht_table_id];

	while (node_id != IDX_NODE_NULL)
	{
		cmp = _is_hash_same(node_id, cur_hash_2, cur_hash_3);
		if (cmp == 0) {
			return IDX_NG_EXIST;
		} else if (cmp < 0) {
			pre_ptr = &(idx_nodes[node_id].left_id);
			node_id = idx_nodes[node_id].left_id;
		} else {
			pre_ptr = &(idx_nodes[node_id].right_id);
			node_id = idx_nodes[node_id].right_id;
		}
	}

	new_node_id=_get_free_idx_node();
	idx_nodes[new_node_id].hash_2 = cur_hash_2;
	idx_nodes[new_node_id].hash_3 = cur_hash_3;
	idx_nodes[new_node_id].disk_loc = disk_location;
	idx_nodes[new_node_id].value_len = value_len;
	idx_nodes[new_node_id].left_id = idx_nodes[new_node_id].right_id = IDX_NODE_NULL;
	*pre_ptr = new_node_id;	
	return IDX_SUCCESS;
}

STATE idx_delete(const char* key, int key_len, PTR_DISK* disk_location)
{
	int ht_table_id, node_id, cmp, nearest_id;
	HASH cur_hash_2, cur_hash_3;
	PTR_KW* pre_ptr;
	PTR_KW* nearest_pre_ptr;
	
	cur_hash_2 = hash_func_2(key, key_len);
	cur_hash_3 = hash_func_3(key, key_len);
	
	ht_table_id = hash_func_1(key, key_len) % HT_TABLE_MOD; /* FIXME: >0 ?? */
	pre_ptr = &ht_table[ht_table_id];
	node_id = ht_table[ht_table_id];

	while ( 1 )
	{
		if (node_id == IDX_NODE_NULL)
			return IDX_NG_NOT_EXIST;

		cmp = _is_hash_same(node_id, cur_hash_2, cur_hash_3);
		if (cmp == 0) {
			break;
		} else if (cmp < 0) {
			pre_ptr = &idx_nodes[node_id].left_id;
			node_id = idx_nodes[node_id].left_id;
		} else {
			pre_ptr = &idx_nodes[node_id].right_id;
			node_id = idx_nodes[node_id].right_id;
		}
	}

	/* idx_nodes[node_id].hash_2 == cur_hash_2 */
	*disk_location = idx_nodes[node_id].disk_loc;
	_put_free_idx_node(node_id);

	if (idx_nodes[node_id].left_id == IDX_NODE_NULL && idx_nodes[node_id].right_id == IDX_NODE_NULL) {
		*pre_ptr = IDX_NODE_NULL;
	} else if (idx_nodes[node_id].left_id != IDX_NODE_NULL && idx_nodes[node_id].right_id != IDX_NODE_NULL) {
		nearest_pre_ptr = &idx_nodes[node_id].right_id;
		nearest_id = idx_nodes[node_id].right_id;
		while (idx_nodes[nearest_id].left_id != IDX_NODE_NULL) {
			nearest_pre_ptr = &idx_nodes[nearest_id].left_id;
			nearest_id = idx_nodes[nearest_id].left_id;
		}

		idx_nodes[nearest_id].left_id = idx_nodes[node_id].left_id;
		*nearest_pre_ptr = idx_nodes[nearest_id].right_id;
		idx_nodes[nearest_id].right_id = idx_nodes[node_id].right_id;
		*pre_ptr = nearest_id;
	} else if (idx_nodes[node_id].left_id != IDX_NODE_NULL) {
		*pre_ptr = idx_nodes[node_id].left_id;		
	} else {
		*pre_ptr = idx_nodes[node_id].right_id;
	}
	return IDX_SUCCESS;
}

static int _is_hash_same(int node_id, HASH hash_2, HASH hash_3)
{
	HASH cur_hash_2, cur_hash_3;
	cur_hash_2=idx_nodes[node_id].hash_2;
	if (hash_2==cur_hash_2) {
		cur_hash_3=idx_nodes[node_id].hash_3;
		if (hash_3 == cur_hash_3) {
			return 0;
		} if (hash_3 < cur_hash_3) {
			return -1;
		} else {
			return 1;
		}
	}
	if (hash_2 < cur_hash_2)
		return -1;
	return 1;
}

static void _dfs(int cur, int _dfs_level);
void  idx_show()
{
	int i;
	printf ("\n---------------------index show----------------------\n");
	
	if (ht_table==NULL || idx_nodes_horizon==NULL || free_idx_nodes_horizon==NULL) {
		printf ("ht_table is null? idx_nodes_horizon is null? free_idx-nodes_horizon is null?\n");
		return;
	}

	printf ("cur idx_nodes_horizon:%d\n",*idx_nodes_horizon);
	printf ("cur free_idx_nodes_horizon:%d\n",*free_idx_nodes_horizon);
	
	for (i=0; i<=HT_TABLE_MOD; i++) {
		if (ht_table[i] == IDX_NODE_NULL)
			continue;
		printf ("table_id:%d\n",i);
		_dfs(ht_table[i],0);
		printf ("\n");
	}
	printf ("-----------------------------------------------------\n");
}

static void _dfs(int cur, int _dfs_level)
{
	int i;
	for (i=0; i<_dfs_level; i++)
		printf ("-");
	if (cur == IDX_NODE_NULL) {
		printf ("\n");
		return;
	}

	printf ("DFS:  [%d  %d] [%d]\n", idx_nodes[cur].hash_2, idx_nodes[cur].hash_3, idx_nodes[cur].disk_loc);

	_dfs(idx_nodes[cur].left_id, _dfs_level+1);
	_dfs(idx_nodes[cur].right_id, _dfs_level+1);
}

static PTR_KW _get_free_idx_node()
{
	/* idx_horizon */
	if ((*free_idx_nodes_horizon)==0) {
		return (*idx_nodes_horizon)++;
	}
	return free_idx_nodes[--(*free_idx_nodes_horizon)];
}

static void _put_free_idx_node(PTR_KW id)
{
	free_idx_nodes[*free_idx_nodes_horizon]=id;
	(*free_idx_nodes_horizon)++;
}

HASH hash_func_1(const char* str, int key_len)
{
	unsigned int hash = 5381;
	int i =0;
	while (i < key_len) {
		hash += (hash << 5) + *(str+i);
		i++;
	}
	return (hash & 0x7FFFFFFF);
}

HASH hash_func_2(const char* str, int key_len)
{
	int hash = 0, x = 0, i = 0;
	while (i < key_len) {
		hash = (hash << 4) + *(str+i);
		if ((x = hash & 0xF0000000L) != 0) {
			hash ^= (x >> 24);
			hash &= ~x;
		}
		i++;
	}
	return hash;
}

HASH hash_func_3(const char* str, int key_len)
{
	int hash = 0, i;
 
	for (i=0; i < key_len; i++) {
		if ((i & 1) == 0) {
			hash ^= ((hash << 7) ^ (*(str+i)) ^ (hash >> 3));
		} else {
			hash ^= (~((hash << 11) ^ (*(str+i)) ^ (hash >> 5)));
		}
	}
	return hash;
}

