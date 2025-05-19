#  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
#  SPDX-License-Identifier: MIT

_EXE=.exe
STATIC=-static

ifeq "$(COMSPEC)" ""
_EXE=
STATIC=
endif

DEBUG=-DDEBUG

CFLAGS=-O2 -Wall -I../src -I. $(STATIC) $(DEBUG)

TESTS_SRC_C=$(wildcard t_*.c)
TESTS_C=$(TESTS_SRC_C:.c=)

TESTS_SRC=$(TST_SRC_C) $(TST_SRC_CPP)
TESTS=$(TESTS_C) $(TESTS_CPP)

# targets
all: $(TESTS)

SRCDIR=../src

runtest: all
	./tstrun.sh

MAKEFLAGS += --no-builtin-rules

%.o: %.c ../src/val.h
	$(CC) $(CFLAGS) -o $*.o -c $< 

%: %.o 
	$(CC) -o $* $< $(LIBS)

.PRECIOUS: %.o

clean:
	rm -f $(TESTS) $(TESTS:=.exe) $(TESTS:=.o) test.log functions.o

