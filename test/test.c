// A simple test program that uses basic operations.
// It opens the graph in test.slg (and empty graph is created if test.slg does not exist),
// lists all nodes and their degrees,
// adds one node, and if the result has at least two nodes, adds an edge from the first to the last node.

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

int main(void)
{
	slgraph_t g;

	// Create or open graph file
	if (slgraph_open(&g, "directed_test.slg", false)) {
		printf("❌ Failed to open directed_test.slg\n");
		return -1;
	}

	// Ensure we're using the correct version
	if (g.version != 2) {
		printf("⚠️  This test requires version 2 (directed graph support). Aborting.\n");
		slgraph_close(&g);
		return -1;
	}

	// Add nodes: A (0), B (1), C (2)
	slgraph_node_t a = slgraph_add_node(&g);
	slgraph_node_t b = slgraph_add_node(&g);
	slgraph_node_t c = slgraph_add_node(&g);

	printf("Added nodes: A=%llu, B=%llu, C=%llu\n",
	       (unsigned long long)a,
	       (unsigned long long)b,
	       (unsigned long long)c);

	// Add directed edges: A → B, B → C
	slgraph_add_directed_edge(&g, a, b);
	slgraph_add_directed_edge(&g, b, c);

	printf("Added directed edges: A → B and B → C\n\n");

	// Print in-degree and out-degree for each node
	for (slgraph_node_t i = 0; i < slgraph_nodes(&g); i++) {
		uint_fast64_t in_deg = slgraph_in_degree(&g, i);
		uint_fast64_t out_deg = slgraph_out_degree(&g, i);
		printf("Node %llu: out-degree = %llu, in-degree = %llu\n",
		       (unsigned long long)i,
		       (unsigned long long)out_deg,
		       (unsigned long long)in_deg);
	}

	printf("\n✅ Degree functions tested.\n\n");

	// Test neighbour access
	for (slgraph_node_t i = 0; i < slgraph_nodes(&g); i++) {
		uint_fast64_t out_deg = slgraph_out_degree(&g, i);
		uint_fast64_t in_deg = slgraph_in_degree(&g, i);

		printf("Node %llu out-neighbours: ", (unsigned long long)i);
		for (uint_fast64_t j = 0; j < out_deg; j++) {
			slgraph_node_t out_nb = slgraph_out_neighbour(&g, i, j);
			printf("%llu ", (unsigned long long)out_nb);
		}
		printf("\n");

		printf("Node %llu in-neighbours: ", (unsigned long long)i);
		for (uint_fast64_t j = 0; j < in_deg; j++) {
			slgraph_node_t in_nb = slgraph_in_neighbour(&g, i, j);
			printf("%llu ", (unsigned long long)in_nb);
		}
		printf("\n");
	}

	printf("\n✅ Neighbour functions tested.\n\n");

	// Test incident edge IDs
	for (slgraph_node_t i = 0; i < slgraph_nodes(&g); i++) {
		uint_fast64_t out_deg = slgraph_out_degree(&g, i);
		uint_fast64_t in_deg = slgraph_in_degree(&g, i);

		printf("Node %llu out-incident edge IDs: ", (unsigned long long)i);
		for (uint_fast64_t j = 0; j < out_deg; j++) {
			uint_fast64_t edge_id = slgraph_out_incident(&g, i, j);
			printf("%llu ", (unsigned long long)edge_id);
		}
		printf("\n");

		printf("Node %llu in-incident edge IDs: ", (unsigned long long)i);
		for (uint_fast64_t j = 0; j < in_deg; j++) {
			uint_fast64_t edge_id = slgraph_in_incident(&g, i, j);
			printf("%llu ", (unsigned long long)edge_id);
		}
		printf("\n");
	}

	printf("\n✅ Incident edge functions tested.\n");

	slgraph_close(&g);
	printf("\n✅ Directed graph test completed.\n");

	return 0;
}
