// A simple program for converting graphs to slgraph format

// Philipp Klaus Krause, philipp@informatik.uni-frankfurt.de, pkk@spth.de, 2014 - 2017
// Copyright (c) 2014-2017 Philipp Klaus Krause
// Copyright (c) 2014-2015 Goethe-Universität Frankfurt
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
#include <string.h>

#include <igraph/igraph.h>

#include "slgraph.h"

void usage(FILE *f)
{
	fprintf(f, "slgraph_convert - convert a grapg to slgraph format.");
	fprintf(f, "Usage: slgraph_convert [--help] [--format <format>] <source> <destination>\n");
	fprintf(f, "Supported formats: edgelist, ncol, graphdb, graphml, gml, pajek.\n");
}

int read_igraph(igraph_t *igraph, const char *format, const char *filename)
{
	int ret;
	FILE *file;

	// Open file
	if(!(file = fopen(filename, "r")))
	{
		printf("Failed to open file %s.\n", filename);
		return(-1);
	}
	
	// Read graph from file
	if(!strcmp("edgelist", format))
		ret = igraph_read_graph_edgelist(igraph, file, 0, false);
	else if(!strcmp("ncol", format))
		ret = igraph_read_graph_ncol(igraph, file, 0, false, IGRAPH_ADD_WEIGHTS_NO, IGRAPH_UNDIRECTED);
	else if(!strcmp("graphdb", format))
		ret = igraph_read_graph_graphdb(igraph, file, false);
	else if(!strcmp("graphml", format))
		ret = igraph_read_graph_graphml(igraph, file, 0);
	else if(!strcmp("gml", format))
		ret = igraph_read_graph_gml(igraph, file);
	else if(!strcmp("pajek", format))
		ret = igraph_read_graph_pajek(igraph, file);
	else
	{
		fclose(file);
		fprintf(stderr, "Unknown file format: %s.\n", format);
		usage(stderr);
		return(-1);
	}

	fclose(file);

	if(ret)
	{
		printf("Failed to read a graph from file %s.\n", filename);
		return(-1);
	}
	
	return(0);
}

int write_slgraph(const char *filename, igraph_t *igraph)
{
	slgraph_t g, h;

	if(slgraph_new(&h))
	{
		printf("Failed to create temporary graph file.\n");
		return(-1);
	}

	if(slgraph_open(&g, filename, false))
	{
		printf("Failed to open destination graph file %s.\n", filename);
		slgraph_close(&h);
		return(-1);
	}

	igraph_integer_t num_nodes = igraph_vcount(igraph);
	slgraph_nodelist_expand(&h, num_nodes);
	for(igraph_integer_t i = 0; i < num_nodes; i++)
		slgraph_add_node(&h);

	igraph_integer_t num_edges = igraph_ecount(igraph);
	for(igraph_integer_t i = 0; i < num_edges; i++)
	{
		igraph_integer_t from, to;
		igraph_edge(igraph, i, &from, &to);
		slgraph_add_edge(&h, from, to);
	}

	igraph_destroy(igraph);

	slgraph_copy(&g, &h); // The copy here helps reduce file size

	slgraph_close(&h);
	slgraph_close(&g);

	return(0);
}

int main(int argc, char *argv[])
{
	const char *format = "none";
	const char *sourcefilename = "";
	const char *destinationfilename = "";

	igraph_t igraph;

	for(int i = 1; i < argc - 2; i++)
	{
		if(!strcmp("--format", argv[i]))
			format = argv[++i];
		else
		{
			usage(!strcmp("--help", argv[i]) ? stdout : stderr);
			return(-1);
		}
	}
	if(argc < 3 || !strncmp("--", argv[argc - 1], 2))
	{
		usage(!strcmp("--help", argv[argc - 1]) ? stdout : stderr);
		return(-1);
	}
	sourcefilename = argv[argc - 2];
	destinationfilename = argv[argc - 1];

	if(read_igraph(&igraph, format, sourcefilename))
	{
		printf("Failed to create input graph.\n");
		return(-1);
	}

	write_slgraph(destinationfilename, &igraph);
}

