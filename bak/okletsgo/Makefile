OBJ= libkvs.so libkvsp.so
CC=gcc
SOCFLAGS=-Wall -fPIC -shared
BINCFLAGS=-Wall -lkvs -lm -lkvsp
SRC_KVS=interface.c sync.c index.c disk.c log.c
SRC_KVSCLIENT=client_func.c
SRC_SERVER=server.c

HEADER=kvs.h kvsp.h
$(OBJ):$(SRC) $(SRC_PROTOCOL)
	$(CC) $(SOCFLAGS) -o libkvs.so $(SRC_KVS)
	$(CC) $(SOCFLAGS) -o libkvsp.so $(SRC_KVSCLIENT)

all:
	make $(OBJ)
	make install
	make server

install:
	mv libkvs.so /usr/local/lib/
	mv libkvsp.so /usr/local/lib/
	cp kvs.h /usr/local/include/
	cp kvsp.h /usr/local/include/
	echo "/usr/local/lib" >> /etc/ld.so.conf
	ldconfig

server: $(SRC_SERVER) 
	$(CC) $(BINCFLAGS) -o server $(SRC_SERVER) -lpthread
	
uninstall:
	rm /usr/local/lib/libkvs.so
	rm /usr/local/lib/libkvsp.so
	rm /usr/local/include/kvs.h
	rm /usr/local/include/kvsp.h
