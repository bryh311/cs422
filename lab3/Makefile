CC=gcc
FLAGS=-g

make: myiptoname mynametoip my_inet_aton

myiptoname: myiptoname.o
	$(CC) myiptoname.o -o myiptoname

myiptoname.o: myiptoname.c
	$(CC) $(FLAGS) myiptoname.c -c

mynametoip: mynametoip.o
	$(CC) $(FLAGS) mynametoip.o -o mynametoip

mynametoip.o: mynametoip.c
	$(CC) $(FLAGS) mynametoip.c -c

my_inet_aton: my_inet_aton.o
	$(CC) my_inet_aton.o -o my_inet_aton

my_inet_aton.o: my_inet_aton.c
	$(CC) $(FLAGS) my_inet_aton.c -c


clean:
	rm mynametoip
	rm myiptoname
	rm my_inet_aton
	rm *.o

