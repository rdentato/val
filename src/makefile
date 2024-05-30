#  SPDX-Creator-Person: Remo Dentato (rdentato@gmail.com)
#  SPDX-License-Identifier: MIT

HDR=val_.h buf_.h vec_.h
SRC_=$(wildcard *.c)
SRC=${SRC_:all.c=}
OBJ=$(SRC:.c=.o)
AR=ar -r
DSTR=../distr

#CFLAGS= -O3 -DNDEBUG -Wall -I./ -I$(DSTR)
CFLAGS= -O2 -DDEBUG -Wall -I./ -I$(DSTR)
#CFLAGS= -pg -O2 -Wall -I./ -I$(DSTR)
#CFLAGS= -g -Wall -I./ -I$(DSTR)


# implicit rules
MAKEFLAGS += --no-builtin-rules

# targets
all: $(DSTR)/val.h $(DSTR)/val.o $(DSTR)/libval.a

$(DSTR)/libval.a:  $(HDR) $(OBJ) 
	$(AR) $(DSTR)/libval.a $(OBJ)

%.o: %.c $(DSTR)/val.h
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: %.o

$(DSTR)/val.h: $(HDR)
	@echo "// `date`" > $(DSTR)/val.h
	@for f in $(HDR); do echo "#line 1 \"$$f\""; cat $$f; done >> $(DSTR)/val.h
	@echo "$(DSTR)/val.h created"

$(DSTR)/val.c: $(HDR)
	@echo "// `date`" > $(DSTR)/val.c
	@for f in $(SRC); do echo "#line 1 \"$$f\""; cat $$f; done >> $(DSTR)/val.c
	@echo "$(DSTR)/val.c created"


$(DSTR)/val.o: $(DSTR)/val.c
	$(CC) $(CFLAGS) -c $< -o $@

clean: 
	rm -f $(DSTR)/libval.a $(DSTR)/val.h $(OBJ)

