# Makefile for Comer CNAI Simple TCP API
# Linux Version

DEFS	= -DLINUX
INC_DIR	= h
INCLUDE = -I$(INC_DIR)
LD	= ld
CC	= gcc
LIBS	= 
FLAGS   = -c -g

CNAIAPI_LIB = cnaiapi.o

all:		 lab2_server lab2_client

APP_EXE = lab1_part_a_client lab1_part_a_server lab1_client lab1_server

API_DIR = api
API_SRC = 	appname_to_appnum.c	\
		cname_to_comp.c		\
		await_contact.c		\
		make_contact.c		\
		readln.c		\
		send_eof.c		\
		cnaiapi_init.c

API_OBJ = $(API_SRC:.c=.o)

lab1_part_a_server: lab1_part_a_server.o $(CNAIAPI_LIB)
	$(CC) lab1_part_a_server.o $(CNAIAPI_LIB) -o lab1_part_a_server

lab1_part_a_server.o:
	$(CC) $(FLAGS) lab1_part_a_server.c $(INCLUDE)

lab1_part_a_client: lab1_part_a_client.o $(CNAIAPI_LIB)
	$(CC) lab1_part_a_client.o $(CNAIAPI_LIB) -o lab1_part_a_client

lab1_part_a_client.o:
	$(CC) $(FLAGS) lab1_part_a_client.c $(INCLUDE)

lab1_server: lab1_server.o
	$(CC) lab1_server.o $(CNAIAPI_LIB) -o lab1_server

lab1_server.o:
	$(CC) $(FLAGS) lab1_server.c $(INCLUDE)

lab1_client: lab1_client.o
	$(CC) lab1_client.o $(CNAIAPI_LIB) -o lab1_client

lab1_client.o:
	$(CC) $(FLAGS) lab1_client.c $(INCLUDE)

$(CNAIAPI_LIB): $(API_OBJ)
	$(LD) -r -o $(CNAIAPI_LIB) $(API_OBJ)

$(API_OBJ):
	$(CC) $(INCLUDE) $(FLAGS) $(DEFS) $(API_DIR)/$(@:.o=.c)

clean:
	\rm -rf *.o core $(CNAIAPI_LIB) $(APP_EXE)

# API Dependancies
appname_to_appnum.o: $(API_DIR)/appname_to_appnum.c $(INC_DIR)/cnaiapi.h

await_contact.o: $(API_DIR)/await_contact.c $(INC_DIR)/cnaiapi.h

cname_to_comp.o: $(API_DIR)/cname_to_comp.c $(INC_DIR)/cnaiapi.h

make_contact.o: $(API_DIR)/make_contact.c $(INC_DIR)/cnaiapi.h

send_eof.o: $(API_DIR)/send_eof.c $(INC_DIR)/cnaiapi.h


