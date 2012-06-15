/*************************************************************************
    > File Name: engine.cc
    > Author: takanoter@gmail.com
*/
//*FIXME this is a hash+tree engine.

#include "kvs_engine.h"
#include "engine.h"

namespace kvs {

HashEngine::HashEngine() {
    //init Writer or sth else memory
}

HashEngine::~HashEngine() {
}

Status HashEngine::Create(const EngineOptions& opt, const std::string& name) {
    fs_path_ = opt.path;
    cabinet_version_ = GetEngineVersion();
    cabinet_name_ = name;
    key_length_ = opt.key_length; 
    arrange_.hole_horizon = opt.hole_horizon;

    Status s = ConfsBorn(opt.path, opt.index_head_size);
    if (!s.ok()) return s;
    s = ConfsSolid();
    if (!s.ok()) return s;

    s = index_.Born(meta_conf_, opt.index_head_size);
    if (!s.ok()) return s;

    s = space_.Born(data_conf_);
    if (!s.ok()) return s;

    health_ = kBorn;
    return s;
}

Status HashEngine::Open(EngineOptions& opt) {
    Status s = ConfsCheckLoad(opt.path);
    if (!s.ok()) return s;

    char buf[32];
    sprintf (buf, "%d", opt.index_head_size);
    std::string v = buf;
    conf_conf_.Set("index_head_size", v); 

    s = index_.Load(meta_conf_, opt.index_head_size);
    if (!s.ok()) return s;

    s = space_.Load(data_conf_);
    if (!s.ok()) return s;
    return s;
}

Status HashEngine::Close() {
    Status s = Solid();
    if (!s.ok()) return s;
    s = data_conf_.Solid();
    return s;
}

Status HashEngine::Put(const PutOptions& opt, const Slice& k, const Slice& v) {
    //id_++;
    Offset off = space_.GetSpace();  //no limit control, out of limit happed in writter.
    Offset len = space_.CalLength(k,v);
    Status s = index_.Insert(opt.cover, k, off, len);
    if (!s.ok()) return s;
    s = space_.Write(off, id_, k, v, opt.sync); 
    if (!s.ok()) goto rollback;
    return s;

rollback:
    if (opt.cover) index_.Backward(); //FIXME only can back one step, may be we can use id_;
    return s;
}

Status HashEngine::Get(const GetOptions& opt, const Slice& k, std::string* v) {
    Offset off, len;
    Status s = index_.Search(k, &off, &len);
    if (!s.ok()) return s;
    if (opt.only_check) return s;  //s.ok() == key is exist
    s = space_.Read(off, len, v);
    return s;
}

Status HashEngine::Delete(const Slice& k) {
    //id++;
    Status s = index_.Del(k);
    //do not deal with space.
    return s;
}

//3 files : index, data, and configure.
//data file never need to be solid, and it should be the newest file.
Status HashEngine::Solid() {
    Status s;

    s = UpdateConfigure(); //dynamic status
    if (!s.ok()) return s;

    s = conf_conf_.Solid(); //FIXME atomic fwrite?
    if (!s.ok()) return s;

    s = meta_conf_.Solid();
    if (!s.ok()) goto rollback;
    return s;

rollback:
//this failed may cause index_file, configure, data_file inconsistency, 
//these 3 files consistency will be checked when open a engine.
// [index_id, data_id] will be recoverd in index file and configure by check data-file.
/*
    s = conf_conf_.Rollback();  
    if (!s.RollbackSuccess()) { //rollback success but solid failed.
         ; //do nothing , do not try again.
    }
*/
    return s;
}

Status HashEngine::Arrange(const ArrangeOptions& opt) {
    Status s;
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

/************  private  **********************************************/
Status HashEngine::ConfsBorn(const std::string& path, const int index_head_size) {
    Status s;
    char filename[1024];

    snprintf(filename, 1024, "%s/%s", path.c_str(), "meta");
    std::string metafile(filename);
    s = meta_conf_.Create(metafile);
    if (!s.ok()) return s;
    s = FillMetaConfigure(meta_conf_);
    if (!s.ok()) return s;

    snprintf(filename, 1024, "%s/%s", path.c_str(), "conf");
    std::string conffile = filename;
    s = conf_conf_.Create(conffile);
    if (!s.ok()) return s;
    s = FillConfConfigure(conf_conf_, index_head_size);
    if (!s.ok()) return s;
     
    snprintf(filename, 1024, "%s/%s", path.c_str(), "data");
    std::string datafile(filename);
    s = data_conf_.Create(datafile);
    if (!s.ok()) return s;
    s = FillDataConfigure(data_conf_);
    if (!s.ok()) return s;

    return s;
}

Status HashEngine::ConfsCheckLoad(std::string& path) {
    Status s; 
    char filename[1024];

    snprintf(filename, 1024, "%s/%s", path.c_str(), "meta");
    std::string metafile(filename);
    s = meta_conf_.Load(metafile);
    if (!s.ok()) return s;

    snprintf(filename, 1024, "%s/%s", path.c_str(), "conf");
    std::string conffile(filename);
    s = conf_conf_.Load(conffile);
    if (!s.ok()) return s;
     
    snprintf(filename, 1024, "%s/%s", path.c_str(), "data");
    std::string datafile(filename);
    s = data_conf_.Load(datafile);
    if (!s.ok()) return s;

    return s;
}

Status HashEngine::ConfsSolid() {
    Status s;
    s = meta_conf_.Solid();
    if (!s.ok()) return s;

    s = conf_conf_.Solid();
    if (!s.ok()) return s;

    s = data_conf_.Solid();
    if (!s.ok()) return s;

    return s;
};

Status HashEngine::FillDataConfigure(CONFIGURE& data) {
    Slice key;
    std::string value;
    char buf[32];
    memset(buf, 0, sizeof(buf));

    key.Set("version");
    Status s = data.NewItem(key, cabinet_version_);
    if (!s.ok()) return s;

    key.Set("name");
    s = data.NewItem(key, cabinet_name_);
    if (!s.ok()) return s;

    key.Set("path");
    s = data.NewItem(key, fs_path_);
    if (!s.ok()) return s;
 
    key.Set("key_length");
    sprintf(buf, "%d", key_length_);
    value = buf;
    s = data.NewItem(key, value);
    if (!s.ok()) return s;

    key.Set("data");
    s = data.NewItem(key);
    if (!s.ok()) return s;

    return s;
}

Status HashEngine::FillConfConfigure(CONFIGURE& conf, const int index_head_size) {
    Slice key;
    std::string value;
    char buf[32];
    memset(buf, 0, sizeof(buf));

    key.Set("version");
    Status s = conf.NewItem(key, cabinet_version_);
    if (!s.ok()) return s;

    key.Set("name");
    s = conf.NewItem(key, cabinet_name_);
    if (!s.ok()) return s;

    key.Set("path");
    s = conf.NewItem(key, fs_path_);
    if (!s.ok()) return s;
 
    key.Set("index_head_size");
    sprintf(buf, "%d", index_head_size);
    value = buf;
    s = conf.NewItem(key, value);
    if (!s.ok()) return s;

    return s;
}

Status HashEngine::FillMetaConfigure(CONFIGURE& meta) {
    Slice key;
    Status s;
    std::string value;
    char buf[32];

    key.Set("version");
    s = meta.NewItem(key, cabinet_version_);
    if (!s.ok()) return s;

    key.Set("name");
    s = meta.NewItem(key, cabinet_name_);
    if (!s.ok()) return s;

    key.Set("key_length");
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%d", key_length_);
    value = buf;
    s = meta.NewItem(key, value);
    if (!s.ok()) return s;

    //dynamic, but static copy
    key.Set("id");
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%lld", id_);
    value = buf;
    s = meta.NewItem(key, value);
    if (!s.ok()) return s;
 
    //dynamic, but static copy
    key.Set("cur_data_file");
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%d", cur_data_file_);
    value = buf;
    s = meta.NewItem(key, value);
    if (!s.ok()) return s;

    //dynamic, but static copy
    key.Set("health");
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%d", health_);
    value = buf;
    s = meta.NewItem(key, value);
    if (!s.ok()) return s;

    //dynamic, but static copy
    key.Set("index_horizon");
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%d", 0);
    value = buf;
    s = meta.NewItem(key, value);
    if (!s.ok()) return s;

    //dynamic, but static copy
    key.Set("index_free_slot_horizon");
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%d", 0);
    value = buf;
    s = meta.NewItem(key, value);
    if (!s.ok()) return s;
 
    key.Set("index_free_slot");
    s = meta.NewItem(key, INDEX_FREE_SLOT_FIX_SIZE);
    if (!s.ok()) return s;

    key.Set("index");
    if (8 == key_length_) {
        s = meta.NewItem(key, INDEX_FIX_8_SIZE);
    } else if (16 == key_length_) {
        s = meta.NewItem(key, INDEX_FIX_16_SIZE);
    } else {
        s.SetInvalidParam("key_length", key_length_);
    }
    if (!s.ok()) return s;
    
    return s;
}

Status HashEngine::UpdateConfigure() {
    Status s;
    Slice key;
    std::string value;
    char buf[32];
    
    key.Set("id");
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%lld", id_);
    value = buf;
    s = meta_conf_.Set(key, value);
    if (!s.ok()) return s;
 
    key.Set("cur_data_file");
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%d", cur_data_file_);
    value = buf;
    s = meta_conf_.Set(key, value);
    if (!s.ok()) return s;

    key.Set("health");
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%d", health_);
    value = buf;
    s = meta_conf_.Set(key, value);
    if (!s.ok()) return s;

    key.Set("index_horizon");
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%lld", index_.GetIndexHorizon());
    value = buf;
    s = meta_conf_.Set(key, value);
    if (!s.ok()) return s;

    key.Set("index_free_slot_horizon");
    memset(buf, 0, sizeof(buf));
    sprintf (buf, "%lld", index_.GetIndexFreeSlotHorizon());
    value = buf;
    s = meta_conf_.Set(key, value);
    if (!s.ok()) return s;

    return s;
}

};  // namespace kvs 

