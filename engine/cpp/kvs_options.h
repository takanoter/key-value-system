/*************************************************************************
    > File Name: kvs_options.h
    > Author: takanoter@gmail.com
*/
#ifndef KVS_ENGINE_INCLUDE_OPTIONS_H_
#define KVS_ENGINE_INCLUDE_OPTIONS_H_

#include <stddef.h>

/*FIXME options will be virtual class in the future to support memory-engine*/
namespace kvs {

struct PutOptions {
  bool cover;
  bool sync;

  PutOptions()
      : cover(false),
        sync(false) {
  }
};

struct GetOptions {
  bool only_check;  //is exist

  GetOptions()
      : only_check(false) {
  }
};

struct ArrangeOptions {
//  const FilterPolicy* filter_policy;
//  const TimeCondition* time_cond;
  const double hole_rate;
  const bool is_check_data;
  const bool persist; //if persist, arragne will do everytime. otherwise it will onle run once.

  ArrangeOptions()
      : //filter_policy(NULL),
        //time_cond(NULL),
        hole_rate(0.0),
        is_check_data(false),
        persist(false) {
  }
};

struct EngineOptions {
 // const Comparator* comparator;
 // bool create_if_missing;
 // bool error_if_exists;
 // bool paranoid_checks;
 // Env* env;
 // const FilterPolicy* filter_policy;
  EngineOptions();
};

};  // namespace kvs

#endif  // KVS_ENGINE_INCLUDE_OPTIONS_H_