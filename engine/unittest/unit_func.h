/*************************************************************************
    > File Name: unit_func.h
    > Author: takanoter@gmail.com
*/
#ifndef KVS_UNITTEST_INCLUDE_UNITFUNC_H_
#define KVS_UNITTEST_INCLUDE_UNITFUNC_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../kvs_slice.h"
#include "../kvs_status.h"
#include "../configure.h"
#include "../index.h"
using namespace kvs;
Status FillDataConfigure(CONFIGURE& data);
Status LoadDataConfigure(CONFIGURE& conf);
Status FillMetaConfigure(CONFIGURE& meta);
Status UpdateMetaConfigure(CONFIGURE& meta_conf, Offset index_horizon, Offset index_free_slot_horizon);


#endif //KVS_UNITTEST_INCLUDE_UNITFUNC_H_
