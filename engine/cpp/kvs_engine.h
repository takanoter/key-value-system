/*************************************************************************
    > File Name: kvs_engine.h
    > Author: takanoter@gmail.com
*/
#ifndef KVS_ENGINE_INCLUDE_ENGINE_H_
#define KVS_ENGINE_INCLUDE_ENGINE_H_

#include <stdio.h>
#include "kvs_status.h"
#include "kvs_options.h"
#include "kvs_slice.h"

namespace kvs {

struct EngineOptions;
struct PutOptions;
struct GetOptions;
struct ArrangeOptions;

class ENGINE {
  public:
//    static Status Close(const Options& options,
//                        ENGINE** engineptr);

    ENGINE() {};
    virtual ~ENGINE();

    virtual Status Create(const EngineOptions& options, const std::string& name) = 0;
    virtual Status Open(const EngineOptions& options) = 0; 
    virtual Status Close() = 0;

    virtual Status Put(const PutOptions& options, 
		       const Slice& key,
                       const Slice& value) = 0;
    virtual Status Get(const GetOptions& options,
                       const Slice& key,
                       std::string* value) = 0;
    virtual Status Delete(const Slice& key) = 0;

    virtual Status Solid() = 0;
    virtual Status Arrange(const ArrangeOptions& options) = 0;
    virtual Status GetArrangeProgress(const int& progress) = 0;
    virtual Status CancelArrange() = 0;

//    virtual Status Version() = 0; //Data Version

  private:
//    DB(const DB&);
//    void operator=(const DB&);
};

static Status DestroyEngine(const std::string& name);

};


#endif // KVS_ENGINE_INCLUDE_ENGINE_H_
