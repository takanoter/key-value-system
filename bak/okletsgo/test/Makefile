
OBJ= client_get client_put lib_get lib_put
CC=gcc
LIB_CFLAGS=-Wall -lkvs -lm
PROTO_CFLAGS=-Wall -lkvsp
SRC= client_get.c client_put.c lib_get.c lib_put.c

$(OBJ):$(SRC) 
	$(CC) -o client_get client_get.c $(PROTO_CFLAGS)
	$(CC) -o client_put client_put.c $(PROTO_CFLAGS)
	$(CC) -o lib_get lib_get.c $(LIB_CFLAGS)
	$(CC) -o lib_put lib_put.c $(LIB_CFLAGS)

clean:
	rm $(OBJ)

