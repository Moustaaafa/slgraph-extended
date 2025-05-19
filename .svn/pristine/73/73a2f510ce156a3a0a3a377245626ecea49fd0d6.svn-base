// A simple program for copying graphs.

// Philipp Klaus Krause, philipp@informatik.uni-frankfurt.de, 2017.
// Copyright (c) 2017 University of Leeds

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

#include <stdbool.h>
#include <stdio.h>

#include "slgraph.h"

int main(int argc, char **argv)
{
	slgraph_t source, destination;

	if(argc != 3)
	{
		fprintf(stderr, "Usage: slgrapg_copy <source> <destination>\n");
		return(-1);
	}

	if(slgraph_open(&source, argv[1], true))
	{
		printf("Failed to open source graph file %s.\n", argv[1]);
		return(-1);
	}

	if(slgraph_open(&destination, argv[2], false))
	{
		printf("Failed to open destination graph file %s.\n", argv[2]);
		return(-1);
	}

	slgraph_copy(&destination, &source);

	slgraph_close(&source);
	slgraph_close(&destination);

	return(0);
}

