CC=gcc
CFLAGS=-Wall -Werror -g -c

make: wireminnow-part4.o
	$(CC) wireminnow-part4.o -o wireminnow-part4

wireminnow-part3.o: wireminnow-part4.c
	$(CC) $(CFLAGS) wireminnow-part4.c

clean:
	rm *.o wireminnow-part4

