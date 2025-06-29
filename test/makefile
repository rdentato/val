#  SPDX-FileCopyrightText: © 2023 Remo Dentato (rdentato@gmail.com)
#  SPDX-License-Identifier: MIT

_EXE=.exe
STATIC=-static

ifeq "$(COMSPEC)" ""
_EXE=
STATIC=
endif

DEBUG=-DDEBUG

SRC=../src
DST=../dist
 
CFLAGS= $(XFLAGS) -O2 -Wall -I$(DST) -I. $(ARCH) $(STATIC) $(DEBUG)
LIBS=-lm

TESTS_SRC=$(wildcard t_*.c)
TESTS_RAW=$(TESTS_SRC:.c=)
TESTS=$(TESTS_SRC:.c=$(_EXE))

# targets
all: $(TESTS)

include $(SRC)/makefile

runtest: dist all
	./tstrun.sh

MAKEFLAGS += --no-builtin-rules

vallib.o: $(DST)/vallib.c  $(DST)/vallib.h 
	$(CC) $(CFLAGS) -o vallib.o -c $(DST)/vallib.c

t_vec: t_vec.o vallib.o
	$(CC) $(ARCH) -s -o t_vec t_vec.o vallib.o $(LIBS) 

%.o: %.c $(SRC)/val.h
	$(CC) $(CFLAGS) -o $*.o -c $< 

%$(_EXE): %.o 
	$(CC) $(ARCH) -s -o $* $< $(LIBS)

%.obj: %.c $(SRC)/val.h
	$(CC) $(ARCH) $(CFLAGS) -o $*.obj -c $< 

.PRECIOUS: %.o %.obj

clean:
	rm -f $(TESTS_RAW) $(TESTS_RAW:=.exe) $(TESTS_RAW:=.o) $(TESTS_RAW:=.obj) test.log functions.o

cleanall: clean	clean_dist
