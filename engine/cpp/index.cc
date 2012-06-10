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
  arrange_.hole_horizon = opt.hole_horizon;

  Status s = ConfsBorn(opt.path);
  if (!s.ok()) return s;
  
  s = index_.Born(opt.index_head_size);
  if (!s.ok()) return s;

  s = space_.Born();
  if (!s.ok()) return s;

  s = health_.Born();
  if (!s.ok()) return s;
  return s;
}

Status HashEngine::Open(const EngineOptions& opt) {
  Status s = ConfsCheckLoad(opt.path);
  if (!s.ok()) return s;
  s = index_.Load(meta_conf_);
  if (!s.ok()) return s;
  s = space_.Load(meta_conf_);
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
    Offset len;
    Offset off = space_.GetSpace();  //no limit control, out of limit happed in writter.
    Status s = index_.Insert(opt.cover, k, off, id_);
    if (!s.ok()) return s;
    s = writer_.Write(off, id_, k, v, len, opt.sync); 
    if (!s.ok()) goto rollback;
    space_.SetUse(len);
    return s;

rollback:
    if (opt.cover) index_.Backward(); //FIXME only can back one step, may be we can use id_;
    return s;
}

Status HashEngine::Get(const GetOptions& opt, const Slice& k, std::string* v) {
    Offset off, len;
    Status s = index_.Search(k, off, len);
    if (!s.ok()) return s;
    if (opt.only_check) return s;  //s.ok() == key is exist
    s = writer_.Read(off, len, v);
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
    index_.Save(meta_conf_);
    space_.Save(meta_conf_);
    if (!meta_conf_.NeedSolid() && !conf_conf_.NeedSolid()) {
        s.SetNoNeed();
        return s;
    }

    s = conf_conf_.Solid(); //FIXME atomic fwrite?
    if (!s.ok()) return s;

    s = meta_conf_.Solid();
    if (!s.ok()) goto rollback;
    return s;

rollback:
//this failed may cause index_file, configure, data_file inconsistency, 
//these 3 files consistency will be checked when open a engine.
// [index_id, data_id] will be recoverd in index file and configure by check data-file.
    s = conf_conf_.Rollback();  
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

