#  SPDX-Creator-Person: Remo Dentato (rdentato@gmail.com)
#  SPDX-License-Identifier: MIT

_EXE=.exe
STATIC=-static

ifeq "$(COMSPEC)" ""
_EXE=
STATIC=
endif

SRCDIR=../../src
DSTR=../../distr

CFLAGS= -O2 -DDEBUG -Wall -I$(DSTR) -I.. $(STATIC)
#CFLAGS= -pg -O2 -Wall -I$(DSTR) -I.. $(STATIC)
#CFLAGS= -g -Wall -I$(DSTR) -I.. $(STATIC)

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
	cd $(SRCDIR) ; make
	$(CC) $(CFLAGS) $< -o $* -L$(DSTR) -lval

#.PRECIOUS: %.o

clean:
	rm -f $(TESTS) $(TESTS:=.exe) $(TESTS:=.o) test.log

cleanall: clean
	make -C $(SRCDIR) clean
