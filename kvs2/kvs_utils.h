
const int err_success = 0;
const int err_malloc_failed = -1;

unsigned long long _kv_consequence_id()
{
    static unsigned long long i = 0;
    return ++i;
}

unsigned long long _get_hashhead_id(unsigned long long key_sign, unsigned long long length)
{
    unsigned long long ret = key_sign % length;
    return ret;
}

