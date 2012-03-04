OBJ=test
DIR=../..
SRC=$(DIR)/index.c $(DIR)/mem_pool.c $(DIR)/sync.c $(DIR)/disk.c $(DIR)/interface.c
CC=gcc
BINCFLAGS=-Wall -I$(DIR)

$(OBJ):
	$(CC) $(BINCFLAGS) -o $(OBJ) $(SRC) ./case/1.c 

1:
	$(CC) $(BINCFLAGS) -o test $(SRC) ./case/1.c -DKVS_TEST
	
clean:
	rm $(OBJ)	
