#ifndef KVS_UTILS
#define KVS_UTILS 1

unsigned long long _kv_consequence_id();

unsigned long long _get_hashhead_id(unsigned long long key_sign, unsigned long long length);

unsigned long long create_key_sign(const char* str, int key_len);

#endif
