// Count strongly connected components in a directed SLGraph.
// Uses a non-recursive Kosaraju-style algorithm.
//
// Usage:
//   slgraph_scc_count <graph.slg>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "slgraph.h"

typedef struct {
	slgraph_node_t node;
	uint64_t next_idx;
} dfs_frame_t;

static int build_finish_order(const slgraph_t *g, slgraph_node_t *order,
                              unsigned char *visited, dfs_frame_t *stack)
{
	uint64_t n = slgraph_nodes(g);
	uint64_t order_len = 0;

	memset(visited, 0, n);

	for (slgraph_node_t start = 0; start < n; start++) {
		uint64_t sp = 0;
		if (visited[start]) {
			continue;
		}

		visited[start] = 1;
		stack[sp++] = (dfs_frame_t){start, 0};

		while (sp > 0) {
			dfs_frame_t *top = &stack[sp - 1];
			uint_fast64_t deg = slgraph_out_degree(g, top->node);

			if (top->next_idx < deg) {
				slgraph_node_t nb = slgraph_out_neighbour(g, top->node, top->next_idx++);
				if (nb != SLGRAPH_INVALID_NODE && !visited[nb]) {
					visited[nb] = 1;
					stack[sp++] = (dfs_frame_t){nb, 0};
				}
				continue;
			}

			order[order_len++] = top->node;
			sp--;
		}
	}

	return 0;
}

static uint64_t count_sccs(const slgraph_t *g, const slgraph_node_t *order,
                           unsigned char *visited, slgraph_node_t *stack,
                           uint64_t *largest)
{
	uint64_t n = slgraph_nodes(g);
	uint64_t count = 0;
	uint64_t max_size = 0;

	memset(visited, 0, n);

	for (uint64_t idx = n; idx > 0; idx--) {
		slgraph_node_t start = order[idx - 1];
		uint64_t sp = 0;
		uint64_t size = 0;

		if (visited[start]) {
			continue;
		}

		visited[start] = 1;
		stack[sp++] = start;

		while (sp > 0) {
			slgraph_node_t v = stack[--sp];
			size++;

			uint_fast64_t deg = slgraph_in_degree(g, v);
			for (uint_fast64_t i = 0; i < deg; i++) {
				slgraph_node_t nb = slgraph_in_neighbour(g, v, i);
				if (nb == SLGRAPH_INVALID_NODE || visited[nb]) {
					continue;
				}
				visited[nb] = 1;
				stack[sp++] = nb;
			}
		}

		if (size > max_size) {
			max_size = size;
		}
		count++;
	}

	if (largest) {
		*largest = max_size;
	}

	return count;
}

int main(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <graph.slg>\n", argv[0]);
		return 1;
	}

	slgraph_t g;
	if (slgraph_open(&g, argv[1], true)) {
		fprintf(stderr, "Failed to open graph: %s\n", argv[1]);
		return 1;
	}

	uint64_t n = slgraph_nodes(&g);
	if (n == 0) {
		fprintf(stderr, "Graph has 0 nodes\n");
		slgraph_close(&g);
		return 1;
	}

	slgraph_node_t *order = malloc(n * sizeof(slgraph_node_t));
	unsigned char *visited = malloc(n * sizeof(unsigned char));
	dfs_frame_t *frames = malloc(n * sizeof(dfs_frame_t));
	slgraph_node_t *stack = malloc(n * sizeof(slgraph_node_t));
	if (!order || !visited || !frames || !stack) {
		fprintf(stderr, "Out of memory for SCC computation\n");
		free(order);
		free(visited);
		free(frames);
		free(stack);
		slgraph_close(&g);
		return 1;
	}

	build_finish_order(&g, order, visited, frames);

	uint64_t largest = 0;
	uint64_t sccs = count_sccs(&g, order, visited, stack, &largest);

	printf("Stats: nodes=%lu edges=%lu mode=scc_count\n",
	       (unsigned long)n, (unsigned long)slgraph_edges(&g));
	printf("SCCS=%lu largest=%lu\n", (unsigned long)sccs, (unsigned long)largest);

	free(order);
	free(visited);
	free(frames);
	free(stack);
	slgraph_close(&g);
	return 0;
}
