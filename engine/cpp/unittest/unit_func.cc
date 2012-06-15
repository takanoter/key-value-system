/*************************************************************************
    > File Name: unit_func.h
    > Author: takanoter@gmail.com
*/
#include "unit_func.h"
Status LoadDataConfigure(CONFIGURE& data_conf) {
    Offset last_offset;
    Status s = data_conf.FetchLastOffset(&last_offset);
    if (!s.ok()) return s;
    int fd = data_conf.GetFD();
    printf ("fd:%d, last_offset:%lld\n", fd, last_offset);

    std::string v;

    s = data_conf.Get("version", &v);
    if (!s.ok()) return s;
    printf ("version:%s\n", v.c_str());

    s = data_conf.Get("name", &v);
    if (!s.ok()) return s;
    printf ("name:%s\n", v.c_str());

    s = data_conf.Get("path", &v);
    if (!s.ok()) return s;
    printf ("path:%s\n", v.c_str());

    s = data_conf.Get("key_length", &v);
    if (!s.ok()) return s;
    int key_length = atoi(v.c_str());
    printf ("key_length:%d\n", key_length); 

    return s;
};

Status FillDataConfigure(CONFIGURE& data) {
    Slice key;
    std::string value;
    char buf[32];
    memset(buf, 0, sizeof(buf));

    key.Set("version");
    Status s = data.NewItem(key, "cabinet_version_");
    if (!s.ok()) return s;

    key.Set("name");
    s = data.NewItem(key, "cabinet_name_");
    if (!s.ok()) return s;

    key.Set("path");
    s = data.NewItem(key, "fs_path_");
    if (!s.ok()) return s;
 
    int key_length_ = 8;
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

Status FillMetaConfigure(CONFIGURE& meta) {
    Slice key;
    Status s;
    std::string value;
    char buf[32];

    key.Set("version");
    s = meta.NewItem(key, "cabinet_version_");
    if (!s.ok()) return s;

    key.Set("name");
    s = meta.NewItem(key, "cabinet_name_");
    if (!s.ok()) return s;

    key.Set("key_length");
    memset(buf, 0, sizeof(buf));
    int key_length_ = 8;
    sprintf(buf, "%d", key_length_);
    value = buf;
    s = meta.NewItem(key, value);
    if (!s.ok()) return s;

    //dynamic, but static copy
    key.Set("id");
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%d", 0);
    value = buf;
    s = meta.NewItem(key, value);
    if (!s.ok()) return s;
 
    //dynamic, but static copy
    key.Set("cur_data_file");
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%d", 0);
    value = buf;
    s = meta.NewItem(key, value);
    if (!s.ok()) return s;

    //dynamic, but static copy
    key.Set("health");
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%d", 0);
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

Status UpdateMetaConfigure(CONFIGURE& meta_conf, Offset index_horizon, Offset index_free_slot_horizon) {
    Slice key;
    char buf[32];
    std::string value;

    key.Set("index_horizon");
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%lld", index_horizon);
    value = buf;
    Status s = meta_conf.Set(key, value);
    if (!s.ok()) return s;

    key.Set("index_free_slot_horizon");
    memset(buf, 0, sizeof(buf));
    sprintf (buf, "%lld", index_free_slot_horizon);
    value = buf;
    s = meta_conf.Set(key, value);
    if (!s.ok()) return s;

    return s;
}
