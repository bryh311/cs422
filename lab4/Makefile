CC = gcc
CFLAGS = -Wall -g

# Target names for the executables
TARGETS = wireminnow-part1 wireminnow-part2 wireminnow-part2-extra

# Object files
OBJ1 = wireminnow-part1.o
OBJ2 = wireminnow-part2.o
OBJ3 = wireminnow-part2-extra.o

all: $(TARGETS)

# Rule to create the first executable
wireminnow-part1: $(OBJ1)
	$(CC) $(CFLAGS) -o $@ $(OBJ1)

# Rule to create the second executable
wireminnow-part2: $(OBJ2)
	$(CC) $(CFLAGS) -o $@ $(OBJ2)

wireminnow-part2-extra: $(OBJ3)
	$(CC) $(CFLAGS) -o $@ $(OBJ3)

# Compile the first object file
wireminnow-part1.o: wireminnow-part1.c
	$(CC) $(CFLAGS) -c wireminnow-part1.c

# Compile the second object file
wireminnow-part2.o: wireminnow-part2.c
	$(CC) $(CFLAGS) -c wireminnow-part2.c

wireminnow-part2-extra.o: wireminnow-part2-extra.c
	$(CC) $(CFLAGS) -c wireminnow-part2-extra.c

# Clean rule to remove the object and executable files
clean:
	rm -f $(TARGETS) $(OBJ1) $(OBJ2) $(OBJ3)

# Add a phony target to avoid conflicts with files named 'clean' or 'all'
.PHONY: all clean

