CC=gcc
CFLAGS=-Wall -Werror -g -c

make: wireminnow-part8.o
	$(CC) wireminnow-part8.o -o wireminnow-part8

wireminnow-part8.o: wireminnow-part8.c
	$(CC) $(CFLAGS) wireminnow-part8.c

clean:
	rm *.o wireminnow-part8

