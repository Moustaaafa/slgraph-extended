.PHONY: all clean

all: slgraph_test slgraph_copy slgraph_convert

LIBFILES = ../include/slgraph.h ../src/slgraph.c

slgraph_test: test.c $(LIBFILES)
	gcc  -O2 -pedantic --std=c11 -D_POSIX_C_SOURCE=200809L -I../include ../src/slgraph.c test.c -o slgraph_test

slgraph_copy: copy.c $(LIBFILES)
	gcc  -O2 -pedantic --std=c11 -D_POSIX_C_SOURCE=200809L -I../include ../src/slgraph.c copy.c -o slgraph_copy

slgraph_convert: convert.c $(LIBFILES)
	gcc  -O2 -pedantic --std=c11 -D_POSIX_C_SOURCE=200809L -I../include ../src/slgraph.c -ligraph convert.c -o slgraph_convert

