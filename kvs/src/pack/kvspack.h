#ifndef VALUE_TYPE_INT
#define VALUE_TYPE_INT  1

namespace kvs {

const int KVS_PACK_HEAD_SIZE = sizeof(int);
class PACK {
  public:
    PACK() {};
    ~PACK() {};
    bool Load(char* buf, int buf_size);
    bool Init(char* buf, int buf_size);
    int Put(char* name, int name_len, int type, char* value, int value_len);
    int Get(char* name, int name_len, int *type, char **value, int *value_len);
    char* GetBuf();
    int GetSendLength();
    void Check();

  private:
    int *send_length_;
    int buf_size_;
    char *buf_; 
}; // class PACK;

}; //namespace kvs
#endif
