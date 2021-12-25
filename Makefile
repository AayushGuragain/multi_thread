CC = gcc
all: defrag

defrag: defrag.c
	$(CC) -g -o defrag defrag.c -lpthread -D_DEFAULT_SOURCE  -std=gnu11 -Wall -Werror
	
clean:
	rm -f defrag output.mp3