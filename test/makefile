#  SPDX-Creator-Person: Remo Dentato (rdentato@gmail.com)
#  SPDX-License-Identifier: MIT

_EXE=.exe
STATIC=-static

ifeq "$(COMSPEC)" ""
_EXE=
STATIC=
endif

CFLAGS=-O2 -Wall -I../src $(STATIC) -DDEBUG
LIBS=-lm

TESTS_SRC=$(wildcard t_*.c)
TESTS=$(TESTS_SRC:.c=)

# implicit rules
MAKEFLAGS += --no-builtin-rules

# targets
all: $(TESTS)

SRCDIR=../src

runtest: all
	@echo > test.out
	@for f in $(TESTS); do echo "TEST: $$f" >> test.out ; ./$$f 2>> test.out; done 
	@echo -n "PASS: `grep 'PASS:' test.out | wc -l`  "
	@echo "FAIL: `grep 'FAIL:' test.out | wc -l`"


%.o: %.c 
	$(CC) $(CFLAGS) -c $< -o $@

%: %.o ../src/tns.o
	$(CC) $(CFLAGS) $< ../src/tns.o -o $* $(LIBS)

.PRECIOUS: %.o

clean:
	rm -f $(TESTS) $(TESTS:=.exe) $(TESTS:=.o) $(SRCDIR)/tns.o test.out

cleanall: clean
	make -C $(SRCDIR) clean
