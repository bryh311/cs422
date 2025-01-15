CC=gcc
CFLAGS=-Wall -Werror -g -c

make: wireminnow-part5.o
	$(CC) wireminnow-part5.o -o wireminnow-part5

wireminnow-part5.o: wireminnow-part5.c
	$(CC) $(CFLAGS) wireminnow-part5.c

clean:
	rm *.o wireminnow-part5

