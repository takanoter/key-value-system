/*************************************************************************
    > File Name: engine.h
    > Author: takanoter@gmail.com
*/
//*FIXME this is a hash+tree engine.
//*FIXME seems that no need to name HashEngine\MemEngine, We can deal that in physics layout?

#ifndef KVS_ENGINE_INCLUDE_HASH_ENGINE_H_
#define KVS_ENGINE_INCLUDE_HASH_ENGINE_H_

//#include "kvs_engine.h"
#include "kvs_status.h"
#include "configure.h"
#include "index.h"
#include "space.h"
//#include "arrange.h"

namespace kvs {

class HashEngine /*: public ENGINE */{
  public:
    HashEngine() {};
    ~HashEngine() {};

    Status Create(const EngineOptions& options, const std::string& name);
    Status Open(EngineOptions& options);
    Status Close();

    // Implementations of the Engine interface
    Status Put(const PutOptions&, const Slice& key, const Slice& value);
    Status Get(const GetOptions&, const Slice& key, std::string* value);
    Status Delete(const Slice& key);
    Status Solid();
    Status Arrange(const ArrangeOptions&);
    Status GetArrangeProgress(const int& progress);
    Status CancelArrange();

  private:
    //*FIXME LOCK LOCK LOCK
   
    std::string cabinet_version_; 
    std::string cabinet_name_; 
    std::string fs_path_;
    int key_length_;

    Offset id_; /*FIXME max operation limit,  consistency control LOCK?*/
	        /*FIXME this will released in User for multithread consideration*/
    int cur_data_file_;
    HealthCode health_;

    //Functions
    ArrangeOptions arrange_;

    //Configure
    CONFIGURE conf_conf_; 
    CONFIGURE meta_conf_; 
    CONFIGURE data_conf_; 
    //we seldom change conf file indeed. Only used for load and set_property+persist, never get_property.
    //because Configure is persist-layout, while specified item in HashEngine is a memory-structure.

    //both in soft-file
    //memory
    INDEX index_; 
    SPACE space_; //data_conf_ manager

  private:
    Status ConfsBorn(const std::string& path, const int hash_head_size);
    Status ConfsCheckLoad(std::string& path);
    Status ConfsSolid();
    Status UpdateConfigure();

    Status FillMetaConfigure(CONFIGURE& meta);
    Status FillConfConfigure(CONFIGURE& conf, const int index_head_size);
    Status FillDataConfigure(CONFIGURE& data);

    Status ArrangeScan(INDEX& base_index, SPACE& base_space ,INDEX& dest_index, SPACE& dest_space);
    std::string GetEngineVersion() {
        return "0.01";
    }

}; // class HashEngine

};  // namespace kvs 

#endif // KVS_ENGINE_INCLUDE_HASH_ENGINE_H_
