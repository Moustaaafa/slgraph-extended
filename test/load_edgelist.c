// Load a large edge list into slgraph format without igraph.
//
// Why this exists:
//   - igraph-based conversion needs lots of RAM for big graphs.
//   - This loader streams the file and builds the slgraph directly.
//
// Input format:
//   - One edge per line: "u v"
//   - Lines starting with '#' or blank lines are ignored.
//
// Node IDs:
//   - Original IDs can be large and sparse (e.g., OSM node IDs).
//   - We remap them to a compact 0..N-1 range for slgraph storage.
//
// Directed vs undirected:
//   - Default is directed edges.
//   - Use --undirected to add edges as undirected.
//
// Usage:
//   slgraph_load_edgelist [--undirected] <input.txt> <output.slg>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "slgraph.h"

static int cmp_u64(const void *a, const void *b) {
	uint64_t va = *(const uint64_t *)a;
	uint64_t vb = *(const uint64_t *)b;
	return (va > vb) - (va < vb);
}

// First pass:
//   Read all edges and collect every node ID in a dynamic array.
//   This is the only data structure we keep in RAM.
static int read_edge_ids(const char *path, uint64_t **out_ids, size_t *out_count) {
	FILE *f = fopen(path, "r");
	if (!f) return -1;

	size_t cap = 1024;
	size_t count = 0;
	uint64_t *ids = malloc(cap * sizeof(uint64_t));
	if (!ids) {
		fclose(f);
		return -1;
	}

	char line[256];
	while (fgets(line, sizeof(line), f)) {
		if (line[0] == '#' || line[0] == '\n') continue;
		uint64_t u, v;
		if (sscanf(line, "%lu %lu", &u, &v) != 2) continue;
		if (count + 2 > cap) {
			cap *= 2;
			uint64_t *tmp = realloc(ids, cap * sizeof(uint64_t));
			if (!tmp) {
				free(ids);
				fclose(f);
				return -1;
			}
			ids = tmp;
		}
		ids[count++] = u;
		ids[count++] = v;
	}

	fclose(f);
	*out_ids = ids;
	*out_count = count;
	return 0;
}

// Sort IDs and remove duplicates so we can map each original ID
// to a compact index using binary search.
static uint64_t *unique_sorted_ids(uint64_t *ids, size_t count, size_t *out_unique) {
	qsort(ids, count, sizeof(uint64_t), cmp_u64);
	if (count == 0) {
		*out_unique = 0;
		return ids;
	}
	size_t w = 1;
	for (size_t i = 1; i < count; i++) {
		if (ids[i] != ids[w - 1]) {
			ids[w++] = ids[i];
		}
	}
	*out_unique = w;
	return ids;
}

// Map an original ID to its compact index in the sorted unique list.
static uint64_t map_id(const uint64_t *ids, size_t n, uint64_t key) {
	const uint64_t *found = bsearch(&key, ids, n, sizeof(uint64_t), cmp_u64);
	if (!found) return UINT64_MAX;
	return (uint64_t)(found - ids);
}

int main(int argc, char **argv) {
	int undirected = 0;
	const char *in_path = NULL;
	const char *out_path = NULL;

	if (argc == 3) {
		in_path = argv[1];
		out_path = argv[2];
	} else if (argc == 4 && strcmp(argv[1], "--undirected") == 0) {
		undirected = 1;
		in_path = argv[2];
		out_path = argv[3];
	} else {
		fprintf(stderr, "Usage: %s [--undirected] <input.txt> <output.slg>\n", argv[0]);
		return 1;
	}

	uint64_t *ids = NULL;
	size_t id_count = 0;
	if (read_edge_ids(in_path, &ids, &id_count)) {
		fprintf(stderr, "Failed to read edge list: %s\n", in_path);
		return 1;
	}

	// Build compact ID mapping.
	size_t unique_count = 0;
	ids = unique_sorted_ids(ids, id_count, &unique_count);
	if (unique_count == 0) {
		fprintf(stderr, "No edges found in: %s\n", in_path);
		free(ids);
		return 1;
	}

	// Create an empty slgraph file and allocate N nodes.
	slgraph_t g;
	if (slgraph_open(&g, out_path, false)) {
		fprintf(stderr, "Failed to open output graph: %s\n", out_path);
		free(ids);
		return 1;
	}

	// Ensure node list capacity and then add nodes.
	slgraph_nodelist_expand(&g, unique_count);
	for (uint64_t i = 0; i < unique_count; i++) {
		if (slgraph_add_node(&g) == SLGRAPH_INVALID_NODE) {
			fprintf(stderr, "Failed to add node %lu\n", (unsigned long)i);
			slgraph_close(&g);
			free(ids);
			return 1;
		}
	}

	FILE *f = fopen(in_path, "r");
	if (!f) {
		fprintf(stderr, "Failed to reopen input: %s\n", in_path);
		slgraph_close(&g);
		free(ids);
		return 1;
	}

	// Second pass: re-read edges, map IDs, and add them to slgraph.
	char line[256];
	while (fgets(line, sizeof(line), f)) {
		if (line[0] == '#' || line[0] == '\n') continue;
		uint64_t u, v;
		if (sscanf(line, "%lu %lu", &u, &v) != 2) continue;
		uint64_t su = map_id(ids, unique_count, u);
		uint64_t sv = map_id(ids, unique_count, v);
		if (su == UINT64_MAX || sv == UINT64_MAX) continue;
		if (undirected) {
			if (slgraph_add_edge(&g, su, sv) == SLGRAPH_INVALID_EDGE) {
				fprintf(stderr, "Failed to add edge %lu -- %lu\n", u, v);
				fclose(f);
				slgraph_close(&g);
				free(ids);
				return 1;
			}
		} else {
			if (slgraph_add_directed_edge(&g, su, sv) == SLGRAPH_INVALID_EDGE) {
				fprintf(stderr, "Failed to add edge %lu -> %lu\n", u, v);
				fclose(f);
				slgraph_close(&g);
				free(ids);
				return 1;
			}
		}
	}

	fclose(f);
	slgraph_close(&g);
	free(ids);
	return 0;
}
