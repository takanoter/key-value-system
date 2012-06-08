/*************************************************************************
    > File Name: engine.h
    > Author: takanoter@gmail.com
*/
//*FIXME this is a hash+tree engine.
//*FIXME seems that no need to name HashEngine\MemEngine, We can deal that in physics layout?

#ifndef KVS_ENGINE_INCLUDE_HASH_ENGINE_H_
#define KVS_ENGINE_INCLUDE_HASH_ENGINE_H_

#include "kvs_engine.h"

namespace kvs {

class HashEngine : public ENGINE {
  public:
    HashEngine(const EngineOptions& options, const std::string& dbname);
    virtual ~HashEngine();

    // Implementations of the Engine interface
    virtual Status Put(const PutOptions&, const Slice& key, const Slice& value);
    virtual Status Get(const GetOptions&, const Slice& key, std::string* value);
    virtual Status Delete(const Slice& key);
    virtual Status Solid();
    virtual Status Arrange(const ArrangeOptions&);
    virtual Status GetArrangeProgress(const int& progress);
    virtual Status CancelArrange();

  private:
    //*FIXME LOCK LOCK LOCK
   
    Version version_; 
    std::string& name_; 
    Offset id_; /*FIXME max item limit,  consistency control LOCK?*/
			    /*FIXME this will released in User for multithread consideration*/

    Writer writer_;
      -->Env env_;

    //both in soft-file
    Index index_;
    Space space_;

    //Functions
    Arrange arrange_;

  private:


}; // class HashEngine

};  // namespace kvs 

#endif // KVS_ENGINE_INCLUDE_HASH_ENGINE_H_
