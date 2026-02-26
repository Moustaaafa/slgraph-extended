// Strong connectivity tester for directed graphs (basic tester).
// Implements Algorithm 1: sample m vertices, run forward/reverse BFS with cutoff L.
//
// Usage:
//   slgraph_tester_basic <graph.slg> <epsilon> <d> [seed]
//
// `d` must be provided as a degree bound > 1.
// to preserve the constant-time (w.r.t. n) implementation model.

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "slgraph.h"

typedef struct {
	uint64_t state;
} rng_t;

static void rng_seed(rng_t *r, uint64_t seed) {
	r->state = seed ? seed : 0x9e3779b97f4a7c15ULL;
}

static uint64_t rng_next(rng_t *r) {
	uint64_t x = r->state;
	x ^= x >> 12;
	x ^= x << 25;
	x ^= x >> 27;
	r->state = x;
	return x * 2685821657736338717ULL;
}

static uint64_t rng_range(rng_t *r, uint64_t n) {
	if (n == 0) return 0;
	uint64_t x, limit = UINT64_MAX - (UINT64_MAX % n);
	do {
		x = rng_next(r);
	} while (x >= limit);
	return x % n;
}

static int queue_contains(const slgraph_node_t *queue, uint64_t tail, slgraph_node_t node)
{
	for (uint64_t i = 0; i < tail; i++) {
		if (queue[i] == node) return 1;
	}
	return 0;
}

// Forward BFS along out-edges with a hard cutoff.
static uint64_t bfs_cutoff_out(const slgraph_t *g, slgraph_node_t start, uint64_t cutoff,
                               slgraph_node_t *queue)
{
	uint64_t head = 0, tail = 0, visited = 0;
	queue[tail++] = start;
	visited = 1;

	while (head < tail && visited < cutoff) {
		slgraph_node_t v = queue[head++];
		uint_fast64_t deg = slgraph_out_degree(g, v);
		for (uint_fast64_t i = 0; i < deg && visited < cutoff; i++) {
			slgraph_node_t nb = slgraph_out_neighbour(g, v, i);
			if (nb == SLGRAPH_INVALID_NODE) continue;
			if (queue_contains(queue, tail, nb)) continue;
			queue[tail++] = nb;
			visited++;
		}
	}
	return visited;
}

// Reverse BFS along in-edges with a hard cutoff.
static uint64_t bfs_cutoff_in(const slgraph_t *g, slgraph_node_t start, uint64_t cutoff,
                              slgraph_node_t *queue)
{
	uint64_t head = 0, tail = 0, visited = 0;
	queue[tail++] = start;
	visited = 1;

	while (head < tail && visited < cutoff) {
		slgraph_node_t v = queue[head++];
		uint_fast64_t deg = slgraph_in_degree(g, v);
		for (uint_fast64_t i = 0; i < deg && visited < cutoff; i++) {
			slgraph_node_t nb = slgraph_in_neighbour(g, v, i);
			if (nb == SLGRAPH_INVALID_NODE) continue;
			if (queue_contains(queue, tail, nb)) continue;
			queue[tail++] = nb;
			visited++;
		}
	}
	return visited;
}

int main(int argc, char **argv) {
	if (argc < 4 || argc > 5) {
		fprintf(stderr, "Usage: %s <graph.slg> <epsilon> <d> [seed]\n", argv[0]);
		return 1;
	}

	const char *path = argv[1];
	double eps = atof(argv[2]);
	uint64_t d = strtoull(argv[3], NULL, 10);
	uint64_t seed = (argc == 5) ? strtoull(argv[4], NULL, 10) : 1;

	if (eps <= 0.0) {
		fprintf(stderr, "epsilon must be > 0\n");
		return 1;
	}

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

	if (d <= 1) {
		fprintf(stderr, "d must be > 1 (automatic degree computation disabled for constant-time mode)\n");
		slgraph_close(&g);
		return 1;
	}

	uint64_t L = (uint64_t)ceil(6.0 / (eps * (double)d));
	uint64_t m = (uint64_t)ceil((6.0 * log(3.0)) / (eps * (double)d));
	if (L < 1) L = 1;
	if (m < 1) m = 1;

	fprintf(stdout, "Stats: nodes=%lu edges=%lu eps=%.6f d=%lu m=%lu L=%lu\n",
	        (unsigned long)n, (unsigned long)slgraph_edges(&g),
	        eps, (unsigned long)d, (unsigned long)m, (unsigned long)L);

	slgraph_node_t *queue = malloc(L * sizeof(slgraph_node_t));
	if (!queue) {
		fprintf(stderr, "Out of memory for BFS structures\n");
		free(queue);
		slgraph_close(&g);
		return 1;
	}

	rng_t rng;
	rng_seed(&rng, seed);

	for (uint64_t t = 0; t < m; t++) {
		slgraph_node_t v = (slgraph_node_t)rng_range(&rng, n);

		uint64_t fwd = bfs_cutoff_out(&g, v, L, queue);

		uint64_t rev = bfs_cutoff_in(&g, v, L, queue);

		if (fwd < L || rev < L) {
			const char *cause = (fwd < L && rev < L) ? "fwd+rev" : (fwd < L ? "fwd" : "rev");
			printf("REJECT (v=%lu, cause=%s, fwd=%lu, rev=%lu, L=%lu)\n",
			       (unsigned long)v, cause,
			       (unsigned long)fwd, (unsigned long)rev, (unsigned long)L);
			free(queue);
			slgraph_close(&g);
			return 0;
		}
	}

	printf("ACCEPT (m=%lu, L=%lu)\n", (unsigned long)m, (unsigned long)L);
	free(queue);
	slgraph_close(&g);
	return 0;
}
