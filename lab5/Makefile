CC=gcc
CFLAGS=-Wall -Werror -g -c

make: wireminnow-part3.o
	$(CC) wireminnow-part3.o -o wireminnow-part3

wireminnow-part3.o: wireminnow-part3.c
	$(CC) $(CFLAGS) wireminnow-part3.c

clean:
	rm *.o wireminnow-part3

