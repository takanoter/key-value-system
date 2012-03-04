OBJ=test
DIR=../..
SRC=$(DIR)/disk.c $(DIR)/mem_pool.c
CC=gcc
BINCFLAGS=-Wall -I$(DIR)

$(OBJ):
	$(CC) $(BINCFLAGS) -o $(OBJ) $(SRC) ./case/1.c 

1:
	$(CC) $(BINCFLAGS) -o test $(SRC) ./case/1.c -DKVS_TEST
	
clean:
	rm $(OBJ)	
