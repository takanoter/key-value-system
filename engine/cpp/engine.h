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
    HashEngine();
    virtual ~HashEngine();

    virtual Status Create(const EngineOptions& options, const std::string& name);
    virtual Status Open(const EngineOptions& options);
    virtual Status Close();

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
   
    Version cabinet_version_; 
    std::string& cabinet_name_; 
    std::string fs_path_;

    Offset id_; /*FIXME max operation limit,  consistency control LOCK?*/
	        /*FIXME this will released in User for multithread consideration*/
    Health health_;

    //Functions
    Arrange arrange_;

    //Configure
    Configure conf_conf_; 
    Configure meta_conf_; 
    Configure data_conf_; 
    //we seldom change conf file indeed. Only used for load and set_property+persist, never get_property.
    //because Configure is persist-layout, while specified item in HashEngine is a memory-structure.

    //both in soft-file
    //memory
    Index index_; 
    Space space_;
  private:


}; // class HashEngine

};  // namespace kvs 

#endif // KVS_ENGINE_INCLUDE_HASH_ENGINE_H_
