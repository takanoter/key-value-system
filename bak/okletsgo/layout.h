#ifndef	KVS_LAYOUT
#define	KVS_LAYOUT

#define	CHUNK_SIZE				(16*1024)

/* "The Big File" (DISK) Layout
 * CHUNK_SIZE oriented (NOT KB or B)
 */ 
#define	DISK_IMAGE				0
#define	DISK_VALUE				34293
#define	DISK_IMAGE_CHUNK_SIZE			34293


/* Image Size in KB
 */
#define	IMAGE_K_SIZE				548684		


/* Image Contains: SuperImage, IndexImage, DiskManagerImage
 */
#define	IMAGE_K_SUPER_SIZE			1	
#define	IMAGE_K_INDEX_SIZE			274341		
#define IMAGE_K_DISK_SIZE			274342	

#define	IMAGE_K_SUPER				0	
#define	IMAGE_K_INDEX				1
#define IMAGE_K_DISK				274342	


/* 1.SuperImage Layout:
 * total 1 KB
 * FIXME: Something may be here in the near future.
 * may be used in function: kv_init() @ interface.c
 * Magic Number? KVS instance version?
 */


/* 2.IndexImage Layout:
 * total 274341 KB
 * Used in function: idx_init() @ index.c
 */
#define	IMAGE_IDX_NODES_HORIZON_SIZE		4
#define	IMAGE_FREE_IDX_NODES_HORIZON_SIZE	4
#define	IMAGE_HT_TABLE_SIZE			524288		/* 131072*4 */	
#define	IMAGE_IDX_NODES_SIZE			280000000	/*FIXME: 7*4*1000*10000 */
#define	IMAGE_FREE_IDX_NODES_SIZE		400096		/*FIXME: 100024*4  10000000*4 */	

#define	IMAGE_IDX_NODES_HORIZON			0
#define	IMAGE_FREE_IDX_NODES_HORIZON		4
#define	IMAGE_HT_TABLE				8
#define	IMAGE_IDX_NODES				524296
#define	IMAGE_FREE_IDX_NODES			280524296


/* 3.DiskManagerImage Layout: 
 * total: 274342 KB
 * 274342KB <-- 274341.484375KB= 280925680B/1024
 * Used in fucntion: disk_init() @ disk.c
 */
#define	IMAGE_D_NODES_HORIZON_SIZE		4
#define	IMAGE_D_NODE_ID_POOL_HORIZON_SIZE	4
#define	IMAGE_D_NODES_SIZE			280000000	/* 1000*10000*7*4 */
#define	IMAGE_D_FREE_LIST_SIZE			1288		/* 322*4 */
#define	IMAGE_D_HASH_SIZE			524288		/* 131072*4  HASH_LENGTH */
#define	IMAGE_D_NODE_ID_POOL_SIZE		400096		/* 100024*4 */

#define	IMAGE_D_NODES_HORIZON			0
#define	IMAGE_D_NODE_ID_POOL_HORIZON		4
#define	IMAGE_D_NODES				8	 
#define	IMAGE_D_FREE_LIST			280000008
#define	IMAGE_D_HASH				280001296
#define	IMAGE_D_NODE_ID_POOL			280525584


#endif

