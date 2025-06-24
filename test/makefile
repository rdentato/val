#  SPDX-FileCopyrightText: © 2023 Remo Dentato (rdentato@gmail.com)
#  SPDX-License-Identifier: MIT

_EXE=.exe
STATIC=-static

ifeq "$(COMSPEC)" ""
_EXE=
STATIC=
endif

DEBUG=-DDEBUG
 
CFLAGS= $(XFLAGS) -O2 -Wall -I../src -I. $(ARCH) $(STATIC) $(DEBUG)
LIBS=-lm

TESTS_SRC=$(wildcard t_*.c)
TESTS_RAW=$(TESTS_SRC:.c=)
TESTS=$(TESTS_SRC:.c=$(_EXE))

# targets
all: $(TESTS)

SRCDIR=../src

runtest: all
	./tstrun.sh

MAKEFLAGS += --no-builtin-rules

%.o: %.c ../src/val.h
	$(CC) $(CFLAGS) -o $*.o -c $< 

%$(_EXE): %.o 
	$(CC) $(ARCH) -s -o $* $< $(LIBS)

%.obj: %.c ../src/val.h
	$(CC) $(ARCH) $(CFLAGS) -o $*.obj -c $< 

.PRECIOUS: %.o %.obj

clean:
	rm -f $(TESTS_RAW) $(TESTS_RAW:=.exe) $(TESTS_RAW:=.o) $(TESTS_RAW:=.obj) test.log functions.o

