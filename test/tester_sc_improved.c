// Strong connectivity tester for directed graphs (improved tester).
// Implements Algorithm 2 with a doubling cutoff. We run both forward and
// reverse BFS to handle directed strong connectivity.
//
// Usage:
//   slgraph_tester_improved <graph.slg> <epsilon> <d_or_0> [seed]
//
// If d_or_0 == 0, the program computes a max degree bound as
// max(out_degree + in_degree) over all nodes.

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

// Forward BFS along out-edges with a hard cutoff.
static uint64_t bfs_cutoff_out(const slgraph_t *g, slgraph_node_t start, uint64_t cutoff,
                               uint32_t *mark, uint32_t *cur_mark, slgraph_node_t *queue)
{
	uint64_t head = 0, tail = 0, visited = 0;
	queue[tail++] = start;
	mark[start] = *cur_mark;
	visited = 1;

	while (head < tail && visited < cutoff) {
		slgraph_node_t v = queue[head++];
		uint_fast64_t deg = slgraph_out_degree(g, v);
		for (uint_fast64_t i = 0; i < deg && visited < cutoff; i++) {
			slgraph_node_t nb = slgraph_out_neighbour(g, v, i);
			if (nb == SLGRAPH_INVALID_NODE) continue;
			if (mark[nb] == *cur_mark) continue;
			mark[nb] = *cur_mark;
			queue[tail++] = nb;
			visited++;
		}
	}
	return visited;
}

// Reverse BFS along in-edges with a hard cutoff.
static uint64_t bfs_cutoff_in(const slgraph_t *g, slgraph_node_t start, uint64_t cutoff,
                              uint32_t *mark, uint32_t *cur_mark, slgraph_node_t *queue)
{
	uint64_t head = 0, tail = 0, visited = 0;
	queue[tail++] = start;
	mark[start] = *cur_mark;
	visited = 1;

	while (head < tail && visited < cutoff) {
		slgraph_node_t v = queue[head++];
		uint_fast64_t deg = slgraph_in_degree(g, v);
		for (uint_fast64_t i = 0; i < deg && visited < cutoff; i++) {
			slgraph_node_t nb = slgraph_in_neighbour(g, v, i);
			if (nb == SLGRAPH_INVALID_NODE) continue;
			if (mark[nb] == *cur_mark) continue;
			mark[nb] = *cur_mark;
			queue[tail++] = nb;
			visited++;
		}
	}
	return visited;
}

// Compute a max degree bound as out_degree + in_degree over all nodes.
static uint64_t compute_max_degree(const slgraph_t *g) {
	uint64_t n = slgraph_nodes(g);
	uint64_t maxd = 0;
	for (uint64_t i = 0; i < n; i++) {
		uint64_t d = slgraph_out_degree(g, i) + slgraph_in_degree(g, i);
		if (d > maxd) maxd = d;
	}
	return maxd;
}

int main(int argc, char **argv) {
	if (argc < 4 || argc > 5) {
		fprintf(stderr, "Usage: %s <graph.slg> <epsilon> <d_or_0> [seed]\n", argv[0]);
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

	if (d == 0) {
		d = compute_max_degree(&g);
		if (d == 0) {
			fprintf(stderr, "Max degree is 0 (empty edge set)\n");
			slgraph_close(&g);
			return 1;
		}
		fprintf(stdout, "Computed max degree d=%lu\n", (unsigned long)d);
	}

	double log_term = log(8.0 / (eps * (double)d));
	if (log_term < 1.0) log_term = 1.0;
	uint64_t iterations = (uint64_t)ceil(log_term);

	fprintf(stdout, "Stats: nodes=%lu edges=%lu eps=%.6f d=%lu iterations=%lu\n",
	        (unsigned long)n, (unsigned long)slgraph_edges(&g),
	        eps, (unsigned long)d, (unsigned long)iterations);

	uint32_t *mark = calloc(n, sizeof(uint32_t));
	if (!mark) {
		fprintf(stderr, "Out of memory for visited marks\n");
		slgraph_close(&g);
		return 1;
	}

	rng_t rng;
	rng_seed(&rng, seed);

	uint32_t cur_mark = 1;
	for (uint64_t i = 1; i <= iterations; i++) {
		uint64_t cutoff = 1ULL << i;
		double denom = (double)cutoff * eps * (double)d;
		uint64_t mi = (uint64_t)ceil(32.0 * log_term / denom);
		if (mi < 1) mi = 1;

		slgraph_node_t *queue = malloc(cutoff * sizeof(slgraph_node_t));
		if (!queue) {
			fprintf(stderr, "Out of memory for BFS queue (cutoff=%lu)\n", (unsigned long)cutoff);
			free(mark);
			slgraph_close(&g);
			return 1;
		}

		for (uint64_t sidx = 0; sidx < mi; sidx++) {
			slgraph_node_t s = (slgraph_node_t)rng_range(&rng, n);

			uint64_t fwd = bfs_cutoff_out(&g, s, cutoff, mark, &cur_mark, queue);
			cur_mark++;
			if (cur_mark == 0) {
				memset(mark, 0, n * sizeof(uint32_t));
				cur_mark = 1;
			}

			uint64_t rev = bfs_cutoff_in(&g, s, cutoff, mark, &cur_mark, queue);
			cur_mark++;
			if (cur_mark == 0) {
				memset(mark, 0, n * sizeof(uint32_t));
				cur_mark = 1;
			}

			if (fwd < cutoff || rev < cutoff) {
				const char *cause = (fwd < cutoff && rev < cutoff) ? "fwd+rev"
				                   : (fwd < cutoff ? "fwd" : "rev");
				printf("REJECT (s=%lu, cause=%s, cutoff=%lu, fwd=%lu, rev=%lu)\n",
				       (unsigned long)s, cause, (unsigned long)cutoff,
				       (unsigned long)fwd, (unsigned long)rev);
				free(queue);
				free(mark);
				slgraph_close(&g);
				return 0;
			}
		}

		free(queue);
	}

	printf("ACCEPT (iterations=%lu)\n", (unsigned long)iterations);
	free(mark);
	slgraph_close(&g);
	return 0;
}
