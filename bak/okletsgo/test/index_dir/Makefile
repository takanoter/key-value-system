OBJ=test
DIR=../..
SRC=$(DIR)/index.c $(DIR)/log.c
CC=gcc
BINCFLAGS=-Wall -I$(DIR)

$(OBJ):
	$(CC) $(BINCFLAGS) -o $(OBJ) $(SRC) ./case/1.c 

1:
	$(CC) $(BINCFLAGS) -o test $(SRC) ./case/1.c -DKVS_TEST
	
0:
	$(CC) $(BINCFLAGS) -o test $(SRC) ./case/0.c -DKVS_TEST

clean:
	rm $(OBJ)	
