CC=gcc
CFLAGS=-Wall -Werror -g -c

make: wireminnow-part6.o
	$(CC) wireminnow-part6.o -o wireminnow-part6

wireminnow-part6.o: wireminnow-part6.c
	$(CC) $(CFLAGS) wireminnow-part6.c

clean:
	rm *.o wireminnow-part6

