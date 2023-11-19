#  SPDX-Creator-Person: Remo Dentato (rdentato@gmail.com)
#  SPDX-License-Identifier: MIT

_EXE=.exe
STATIC=-static

ifeq "$(COMSPEC)" ""
_EXE=
STATIC=
endif

CFLAGS=-O2 -Wall -I../../src -I.. $(STATIC) -DDEBUG
LIBS=

TESTS_SRC=$(wildcard t_*.c)
TESTS=$(TESTS_SRC:.c=)

# implicit rules
MAKEFLAGS += --no-builtin-rules

# targets
all: $(TESTS)

SRCDIR=../src

runtest: all
	@../tstrun 

#%.o: %.c 
#	$(CC) $(CFLAGS) -c $< -o $@

%: %.c 
	$(CC) $(CFLAGS) $< -o $* $(LIBS)

#.PRECIOUS: %.o

clean:
	rm -f $(TESTS) $(TESTS:=.exe) $(TESTS:=.o) test.log

cleanall: clean
	make -C $(SRCDIR) clean
