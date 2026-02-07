.PHONY: all clean

all: slgraph_test slgraph_copy slgraph_convert slgraph_load_edgelist

LIBFILES = ../include/slgraph.h ../src/slgraph.c

PKG_CONFIG ?= pkg-config
IGRAPH_CFLAGS := $(shell $(PKG_CONFIG) --cflags igraph 2>/dev/null)
IGRAPH_LIBS := $(shell $(PKG_CONFIG) --libs igraph 2>/dev/null)
XML2_LIBS := $(shell $(PKG_CONFIG) --libs libxml-2.0 2>/dev/null)

slgraph_test: test.c $(LIBFILES)
	gcc  -O2 -pedantic --std=c11 -D_POSIX_C_SOURCE=200809L -I../include ../src/slgraph.c test.c -o slgraph_test

slgraph_copy: copy.c $(LIBFILES)
	gcc  -O2 -pedantic --std=c11 -D_POSIX_C_SOURCE=200809L -I../include ../src/slgraph.c copy.c -o slgraph_copy

slgraph_convert: convert.c $(LIBFILES)
	gcc  -O2 -pedantic --std=c11 -D_POSIX_C_SOURCE=200809L -I../include $(IGRAPH_CFLAGS) ../src/slgraph.c convert.c -o slgraph_convert $(IGRAPH_LIBS) $(XML2_LIBS) -lm

slgraph_load_edgelist: load_edgelist.c $(LIBFILES)
	gcc  -O2 -pedantic --std=c11 -D_POSIX_C_SOURCE=200809L -I../include ../src/slgraph.c load_edgelist.c -o slgraph_load_edgelist
