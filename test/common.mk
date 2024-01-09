#  SPDX-Creator-Person: Remo Dentato (rdentato@gmail.com)
#  SPDX-License-Identifier: MIT

_EXE=.exe
STATIC=-static

ifeq "$(COMSPEC)" ""
_EXE=
STATIC=
endif

SRCDIR=../../src

CFLAGS=-g -DDEBUG -Wall -I$(SRCDIR) -I.. $(STATIC)

TESTS_SRC=$(wildcard t_*.c)
TESTS=$(TESTS_SRC:.c=)

# implicit rules
MAKEFLAGS += --no-builtin-rules

# targets
all: objs $(TESTS)

runtest: all
	@../tstrun 

objs:
	cd $(SRCDIR) ; make

#%.o: %.c 
#	$(CC) $(CFLAGS) -c $< -o $@

%: %.c 
	$(CC) $(CFLAGS) $< $(OBJ) -o $*

#.PRECIOUS: %.o

clean:
	rm -f $(TESTS) $(TESTS:=.exe) $(TESTS:=.o) test.log

cleanall: clean
	make -C $(SRCDIR) clean
