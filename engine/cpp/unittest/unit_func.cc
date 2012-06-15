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
