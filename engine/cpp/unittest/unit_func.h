/*************************************************************************
    > File Name: unit_func.h
    > Author: takanoter@gmail.com
*/
#ifndef KVS_UNITTEST_INCLUDE_UNITFUNC_H_
#define KVS_UNITTEST_INCLUDE_UNITFUNC_H_

#include <stdio.h>
#include "../kvs_slice.h"
#include "../kvs_status.h"
#include "../configure.h"
using namespace kvs;
Status FillDataConfigure(CONFIGURE& data);
Status LoadDataConfigure(CONFIGURE& conf);



#endif //KVS_UNITTEST_INCLUDE_UNITFUNC_H_
