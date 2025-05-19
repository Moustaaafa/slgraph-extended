#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct slgraph_t
{
	int fd;
	bool readonly;
	unsigned char *ptr;
	size_t size;
	size_t free;
	uint64_t version;
};

typedef struct slgraph_t slgraph_t;
typedef uint_fast64_t slgraph_node_t;
typedef uint_fast64_t slgraph_edge_t;

#define SLGRAPH_INVALID_NODE UINT_FAST64_MAX
#define SLGRAPH_INVALID_EDGE UINT_FAST64_MAX

// Helper to write 6-byte little-endian integers
void write_6_bytes(unsigned char *dst, uint64_t value);

// Helper to read 6-byte little-endian integers
uint64_t read_6_bytes(const unsigned char *src);

// Internal helpers to read/write 48-bit and 64-bit integers
uint_fast64_t slgraph_read48(const unsigned char *ptr);
void slgraph_write48(unsigned char *ptr, uint_fast64_t v);
uint_fast64_t slgraph_read64(const unsigned char *ptr);
void slgraph_write64(unsigned char *ptr, uint_fast64_t v);

// Create a new, empty graph. Returns 0 if successful.
int slgraph_new(slgraph_t *g);

// Open the file at filename as g. Returns 0 if successful. Complexity O(1).
int slgraph_open(slgraph_t *g, const char *restrict filename, bool readonly);

// Reserve space for up to a total of n nodes.
// Returns 0 if successful. Complexity O(slgraph_nodes()).
int slgraph_nodelist_expand(slgraph_t *g, uint_fast64_t n);

// Close g. Ensure the underlying file is in a consistent state. Complexity O(1).
void slgraph_close(slgraph_t *g);

// Make g a copy of h. Returns 0 if successful. Complexity O(nodes + edges).
int slgraph_copy(slgraph_t *g, const slgraph_t *h);

// Get the number of nodes in g. Complexity O(1).
uint_fast64_t slgraph_nodes(const slgraph_t *g);

// Get the number of edges in g. Complexity O(1).
uint_fast64_t slgraph_edges(const slgraph_t *g);

// Get the degree of n in g. Complexity O(1).
uint_fast64_t slgraph_degree(const slgraph_t *g, slgraph_node_t n);

// Get the i-th neighbour of n in g (SLGRAPH_INVALID_NODE if none). Complexity O(1).
slgraph_node_t slgraph_neighbour(const slgraph_t *g, slgraph_node_t n, uint_fast32_t i);

// Get the i-th incident edge of n in g. Complexity O(1).
slgraph_edge_t slgraph_incident(const slgraph_t *g, slgraph_node_t n, uint_fast32_t i);

// Get the endpoints of edge e in g. Complexity O(1).
void slgraph_edge_ends(const slgraph_t *g, slgraph_edge_t e, slgraph_node_t *n0, slgraph_node_t *n1);

// Add a node to g. Returns new node or SLGRAPH_INVALID_NODE on error.
slgraph_node_t slgraph_add_node(slgraph_t *g);

// Add an undirected edge from n0 to n1. Returns new edge or SLGRAPH_INVALID_EDGE.
slgraph_edge_t slgraph_add_edge(slgraph_t *g, slgraph_node_t n0, slgraph_node_t n1);

// === Directed Graph Support ===

#pragma pack(push, 1)
typedef struct {
    uint64_t out_offset;
    uint64_t in_offset;
    uint8_t label[6];
} slgraph_node_entry_v2;
#pragma pack(pop)

// Add a directed edge from src to dst. Returns edge ID or SLGRAPH_INVALID_EDGE.
uint_fast64_t slgraph_add_directed_edge(slgraph_t *g, uint_fast64_t src, uint_fast64_t dst);

uint_fast64_t slgraph_out_degree(const slgraph_t *g, slgraph_node_t n);
uint_fast64_t slgraph_in_degree(const slgraph_t *g, slgraph_node_t n);

slgraph_edge_t slgraph_out_incident(const slgraph_t *g, slgraph_node_t n, uint_fast32_t i);
slgraph_edge_t slgraph_in_incident(const slgraph_t *g, slgraph_node_t n, uint_fast32_t i);

slgraph_node_t slgraph_out_neighbour(const slgraph_t *g, slgraph_node_t n, uint_fast32_t i);
slgraph_node_t slgraph_in_neighbour(const slgraph_t *g, slgraph_node_t n, uint_fast32_t i);

// === Internal accessors ===

// Get pointer to node list
static unsigned char *slgraph_nodelist(const slgraph_t *g);

// Get pointer to edge list
static unsigned char *slgraph_edgelist(const slgraph_t *g);

