#  SPDX-FileCopyrightText: © 2023 Remo Dentato (rdentato@gmail.com)
#  SPDX-License-Identifier: MIT

DST=../dist
SRC=../src

dist: $(DST)/val.h $(DST)/vallib.h $(DST)/vallib.c

$(DST)/vallib.h: $(SRC)/val.h $(SRC)/val_lib_pre.h $(SRC)/val_lib_post.h
	cat $(SRC)/val_lib_pre.h $(SRC)/val.h $(SRC)/val_lib_post.h > $(DST)/vallib.h

$(DST)/vallib.c: $(SRC)/vallib.c
	cp $(SRC)/vallib.c ../dist

$(DST)/val.h: $(SRC)/val.h
	cp $(SRC)/val.h ../dist

clean_dist:
	rm -f $(DST)/val*
