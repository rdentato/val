#  SPDX-Creator-Person: Remo Dentato (rdentato@gmail.com)
#  SPDX-License-Identifier: MIT

CFLAGS= -g -DDEBUG -Wall -I./

SRC=$(wildcard *.c)
OBJ=$(SRC:.c=.o)

# implicit rules
MAKEFLAGS += --no-builtin-rules

# targets
all: $(OBJ)

%.o: %.c 
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: %.o

clean:
	rm -f $(OBJ) 

