CC=gcc
CFLAGS=-Wall -Werror -g -c

make: wireminnow-part7.o
	$(CC) wireminnow-part7.o -o wireminnow-part7

wireminnow-part7.o: wireminnow-part7.c
	$(CC) $(CFLAGS) wireminnow-part7.c

clean:
	rm *.o wireminnow-part7

