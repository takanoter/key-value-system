/*************************************************************************
    > File Name: engine.cc
    > Author: takanoter@gmail.com
*/
//*FIXME this is a hash+tree engine.

#include "kvs_engine.h"
#include "engine.h"

namespace kvs {

HashEngine::HashEngine(const EngineOptions& opt, const std::string& name) {




}

HashEngine::~HashEngine() {


}

Status HashEngine::Put(const PutOptions& opt, const Slice& k, const Slice& v) {
    //id_++;
    Offset len;
    Offset off = space_.GetSpace();  //no limit control, out of limit happed in writter.
    Status s = index_.insert(opt.cover, k, off, id_);
    if (!s.ok()) return s;
    s = writer_.write(off, id_, k, v, len, opt.sync); 
    if (!s.ok()) goto rollback;
    space_.SetUse(len);
    return s;

rollback:
    if (opt.cover) index_.backward(); //FIXME only can back one step, may be we can use id_;
    return s;
}

Status HashEngine::Get(const GetOptions& opt, const Slice& k, std::string* v) {
    Offset off, len;
    Status s = index_.search(k, off, len);
    if (!s.ok()) return s;
    if (opt.only_check) return s;  //s.ok() == key is exist
    s = writer_.read(off, len, v);
    return s;
}

Status HashEngine::Delete(const Slice& k) {
    //id++;
    Status s = index_.del(k);
    //do not deal with space.
    return s;
}

//3 files : index, data, and configure.
//data file never need to be solid, and it should be the newest file.

Status HashEngine::Solid() {
    Status s;
    if (solid_id_ == id_) return s;
    s = writer_.sync_soft(env_.idx_fd, id_); //FIXME atomic write
    if (!s.ok()) return s;

    Configure conf(env_);
    //configure is a soft-conf-file, it can be changed in runtime, but never hot-load.
    //check whether it changed, and refresh conf-file after BACKUP
    s = writer_.sync_conf(env_.conf_fd, conf); //FIXME atomic write
    if (!s.ok()) goto rollback;
    solid_id_ = id_;
    return s;

rollback:
//this failed may cause index_file, configure, data_file inconsistency, 
//these 3 files consistency will be checked when open a engine.
// [index_id, data_id] will be recoverd in index file and configure by check data-file.
    s = writer_.sync_soft_rollback();  
    if (!s.RollbackSuccess()) { //rollback success but solid failed.
         ; //do nothing , do not try again.
    }
    return s;
}

Status HashEngine::Arrange(const ArrangeOptions& opt) {

}

Status HashEngine::GetArrangeProgress(const int& prog) {


}

Status HashEngine::CancelArrange() {

}

};  // namespace kvs 

