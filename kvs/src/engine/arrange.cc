/*************************************************************************
    > File Name: arrange.h
    > Author: takanoter@gmail.com
*/
#ifndef KVS_ENGINE_INCLUDE_ARRANGE_H_
#define KVS_ENGINE_INCLUDE_ARRANGE_H_

#include "engine.h"

namespace kvs {

Status HashEngine::Arrange(const ArrangeOptions& opt) {
    Status s;
    /*
    1.new meta_conf.1 (conf), new INDEX.1
    2.health or status == arrange. 
      INCOMING:data: data.0
               index: both
    3.scan data.0 ,check with index.0, then
      write to data.1, index.1
    4.health = changing...
      INDEX.load(meta_conf_.load(meta.1)), SPACE.load(data_conf_.load(data.1))
    5.cur_data_file = 1;
      Solid();
      health = kBorn;

    */
    health_ = kArrange; 
    int new_data_file = (cur_data_file_ + 1) % 2;
    char filename[1024];
    char buffer[1024];

    CONFIGURE new_meta_conf;
    memset(filename, 0, sizeof(filename));
    sprintf(filename, "%s/meta.%d", fs_path_.c_str(), new_data_file);
    s = new_meta_conf.Create(filename);
    if (!s.ok()) {health_ = kBorn; return s;}
    s = meta_conf_.Reborn(new_meta_conf); //DO NOT Solid inside here!!!!!!!!!
    if (!s.ok()) {health_ = kBorn; return s;}

    //Reset new_meta_conf_:index_horizon, index_free_slot_horizon
    Slice key, value;
    std::string v;
   
    key.Set("index_horizon");
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "%d", 0); //set to 0, not index_.GetIndexHorizon()
    v = buffer;
    value.Set(v.c_str());
    s = new_meta_conf.Set(key, value);
    if (!s.ok()) return s;

    key.Set("index_free_slot_horizon");
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "%d", 0); // set to 0, not index_.GetIndexFreeSlotHorizon()
    v = buffer;
    value.Set(v.c_str());
    s = new_meta_conf.Set(key, value);
    if (!s.ok()) return s;


    INDEX new_index;
    new_index.Born(new_meta_conf, index_.hash_head_size_);

    CONFIGURE new_data_conf;
    memset(filename, 0, sizeof(filename));
    sprintf(filename, "%s/data.%d", fs_path_.c_str(), new_data_file);
    s = new_data_conf.Create(filename);
    if (!s.ok()) {health_ = kBorn; return s;}
    s = data_conf_.Reborn(new_data_conf); //Do NOT Solid inside here!!!!!!!!!
    //Reset data_conf_: NOTHING

    s = new_data_conf.Solid();
    if (!s.ok()) return s;
    SPACE new_space;
    s = new_space.Born(new_data_conf);
    if (!s.ok()) return s;

    s = ArrangeScan(index_, space_, new_index, new_space);
    if (!s.ok()) return s;
    s = new_meta_conf.Solid();
    if (!s.ok()) return s;
    s = new_data_conf.Solid();
    if (!s.ok()) return s;


    health_ = kJump;
    
    memset(filename, 0, sizeof(filename));
    sprintf(filename, "%s/meta.%d", fs_path_.c_str(), new_data_file);
    s = meta_conf_.Load(filename);
    if (!s.ok()) return s;
    s = index_.Load(meta_conf_, index_.hash_head_size_);
    if (!s.ok()) return s;

    memset(filename, 0, sizeof(filename));
    sprintf(filename, "%s/data.%d", fs_path_.c_str(), new_data_file);
    s = data_conf_.Load(filename);
    if (!s.ok()) return s;
    s = space_.Load(data_conf_);
    if (!s.ok()) return s;
    
    cur_data_file_ = new_data_file;
    health_ = kBorn;
    
    s = Solid();
    if (!s.ok()) return s;
    return s;
}

Status HashEngine::ArrangeScan(INDEX& base_index, SPACE& base_space, 
                               INDEX& dest_index, SPACE& dest_space) {
    Status s;
    Offset base_off = dest_space.GetSpace();
    Offset key_length = key_length_;
    std::string key, value;
    Offset kv_length = 0;
    
    Offset new_id = 0;
    while (base_off < base_space.GetSpace()) {
        //scan from base_conf
        s = base_space.Read(base_off, key_length, &key, &value, &kv_length);
        if (!s.ok()) return s;
        Slice k(key);
        Slice v(value);

        //check with base_index
        Offset index_off, index_len;
        s = base_index.Search(k, &index_off, &index_len);
        if ( (index_off!=base_off) || (index_len!=kv_length) ) {
            base_off += kv_length;
            continue;
        }

        //insert to dest_index
        Offset dest_conf_off = dest_space.GetSpace();
        s = dest_index.Insert(true, k, dest_conf_off, kv_length);
        if (!s.ok()) return s;

        //write to dest_conf
        s = dest_space.Write(dest_conf_off, new_id++, k, v, false);
        if (!s.ok()) return s;
        base_off += kv_length;
    }

    return s;
}

Status HashEngine::GetArrangeProgress(const int& prog) {
    Status s;
    return s;
}

Status HashEngine::CancelArrange() {
    Status s;
    return s;
}

}  // namespace kvs

#endif // KVS_ENGINE_INCLUDE_ARRANGE_H_
