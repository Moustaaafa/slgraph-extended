// Classical strong connectivity checker for directed graphs.
// Uses full-size BFS with a visited array over all n vertices.
//
// Usage:
//   slgraph_tester_classical <graph.slg>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "slgraph.h"

static uint64_t bfs_full_out(const slgraph_t *g, slgraph_node_t start,
                             unsigned char *visited, slgraph_node_t *queue)
{
	uint64_t nvisited = 0;
	uint64_t head = 0, tail = 0;

	memset(visited, 0, slgraph_nodes(g));
	visited[start] = 1;
	queue[tail++] = start;
	nvisited = 1;

	while (head < tail) {
		slgraph_node_t v = queue[head++];
		uint_fast64_t deg = slgraph_out_degree(g, v);
		for (uint_fast64_t i = 0; i < deg; i++) {
			slgraph_node_t nb = slgraph_out_neighbour(g, v, i);
			if (nb == SLGRAPH_INVALID_NODE || visited[nb]) {
				continue;
			}
			visited[nb] = 1;
			queue[tail++] = nb;
			nvisited++;
		}
	}

	return nvisited;
}

static uint64_t bfs_full_in(const slgraph_t *g, slgraph_node_t start,
                            unsigned char *visited, slgraph_node_t *queue)
{
	uint64_t nvisited = 0;
	uint64_t head = 0, tail = 0;

	memset(visited, 0, slgraph_nodes(g));
	visited[start] = 1;
	queue[tail++] = start;
	nvisited = 1;

	while (head < tail) {
		slgraph_node_t v = queue[head++];
		uint_fast64_t deg = slgraph_in_degree(g, v);
		for (uint_fast64_t i = 0; i < deg; i++) {
			slgraph_node_t nb = slgraph_in_neighbour(g, v, i);
			if (nb == SLGRAPH_INVALID_NODE || visited[nb]) {
				continue;
			}
			visited[nb] = 1;
			queue[tail++] = nb;
			nvisited++;
		}
	}

	return nvisited;
}

int main(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <graph.slg>\n", argv[0]);
		return 1;
	}

	const char *path = argv[1];
	slgraph_t g;
	if (slgraph_open(&g, path, true)) {
		fprintf(stderr, "Failed to open graph: %s\n", path);
		return 1;
	}

	uint64_t n = slgraph_nodes(&g);
	if (n == 0) {
		fprintf(stderr, "Graph has 0 nodes\n");
		slgraph_close(&g);
		return 1;
	}

	fprintf(stdout, "Stats: nodes=%lu edges=%lu mode=classical\n",
	        (unsigned long)n, (unsigned long)slgraph_edges(&g));

	unsigned char *visited = malloc(n * sizeof(unsigned char));
	slgraph_node_t *queue = malloc(n * sizeof(slgraph_node_t));
	if (!visited || !queue) {
		fprintf(stderr, "Out of memory for classical BFS structures\n");
		free(visited);
		free(queue);
		slgraph_close(&g);
		return 1;
	}

	uint64_t fwd = bfs_full_out(&g, 0, visited, queue);
	if (fwd != n) {
		printf("REJECT (start=0, cause=fwd, reached=%lu, total=%lu)\n",
		       (unsigned long)fwd, (unsigned long)n);
		free(visited);
		free(queue);
		slgraph_close(&g);
		return 0;
	}

	uint64_t rev = bfs_full_in(&g, 0, visited, queue);
	if (rev != n) {
		printf("REJECT (start=0, cause=rev, reached=%lu, total=%lu)\n",
		       (unsigned long)rev, (unsigned long)n);
		free(visited);
		free(queue);
		slgraph_close(&g);
		return 0;
	}

	printf("ACCEPT (start=0, reached=%lu)\n", (unsigned long)n);
	free(visited);
	free(queue);
	slgraph_close(&g);
	return 0;
}
