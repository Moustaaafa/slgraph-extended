# slgraph-extended
Extended fork of slgraph with support for directed graphs, testing utilities, and more experimental features.

## Data Pipeline (OSM/Network -> slgraph)

### 1) Build test tools

```bash
cd test
make slgraph_load_edgelist slgraph_tester_basic
cd ..
```

### 2) Convert input data to `u v` edge list text

Use the helper script:

```bash
chmod +x scripts/prepare_edgelist.sh
```

OSM input (`.osm` or `.osm.pbf`):

```bash
scripts/prepare_edgelist.sh --mode osm --input /path/to/map.osm.pbf --output graph-edges.txt
```

Generic network table input (`.txt/.csv/.tsv`):

```bash
scripts/prepare_edgelist.sh --mode table --input /path/to/graph.csv --output graph-edges.txt --src-col 1 --dst-col 2 --skip-header
```

Optional flags:

```bash
--undirected   # emit both u v and v u, then deduplicate
--dedup        # remove duplicate edges
```

### 3) Load edge list into slgraph binary

Directed (default):

```bash
test/slgraph_load_edgelist graph-edges.txt graph.slg
```

Undirected:

```bash
test/slgraph_load_edgelist --undirected graph-edges.txt graph.slg
```

### 4) Run strong-connectivity tester

```bash
test/slgraph_tester_basic graph.slg 0.05 8 1
```

Arguments:
- `graph.slg`: input slgraph file
- `0.05`: epsilon
- `8`: explicit degree bound `d` (must be > 1)
- `1`: RNG seed (optional)

## Example Run

If you already have `bamberg-edges.txt`:

```bash
cd test
make slgraph_load_edgelist slgraph_tester_basic
cd ..

test/slgraph_load_edgelist bamberg-edges.txt bamberg.slg
test/slgraph_tester_basic bamberg.slg 0.05 8 1
```

If you start from OSM:

```bash
chmod +x scripts/prepare_edgelist.sh
scripts/prepare_edgelist.sh --mode osm --input /path/to/bamberg.osm.pbf --output bamberg-edges.txt

cd test
make slgraph_load_edgelist slgraph_tester_basic
cd ..

test/slgraph_load_edgelist bamberg-edges.txt bamberg.slg
test/slgraph_tester_basic bamberg.slg 0.05 8 1
```
