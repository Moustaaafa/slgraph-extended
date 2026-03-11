# slgraph-extended
Extended fork of slgraph with support for directed graphs, testing utilities, and more experimental features.

## Data Pipeline (OSM/Network -> slgraph)

### 1) Build test tools

```bash
cd test
make slgraph_load_edgelist slgraph_tester_basic slgraph_tester_improved slgraph_tester_classical
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

Classical tester:

```bash
test/slgraph_tester_classical graph.slg
```

Basic tester:

```bash
test/slgraph_tester_basic graph.slg 0.05 8 1
```

Improved tester:

```bash
test/slgraph_tester_improved graph.slg 0.05 8 1
```

Arguments:
- `graph.slg`: input slgraph file
- `0.05`: epsilon
- `8`: explicit degree bound `d` (must be > 1)
- `1`: RNG seed (optional)

The classical tester performs a full-size reachability check using
queue and visited arrays of size \(n\). It checks whether all vertices
are reachable from a fixed start vertex in both the forward and reverse
directions, which yields a standard \(O(n+m)\) strong-connectivity
decision rather than a bounded-query property test.

### 5) Run both testers over multiple seeds

Use the helper script:

```bash
python3 run_20_tests.py graph.slg 0.05 8 1 20
```

What it does:
- runs `test/slgraph_tester_basic` once per seed in the range `1..20`
- runs `test/slgraph_tester_improved` once per seed in the range `1..20`
- prints the final `ACCEPT` or `REJECT` line for each run
- prints total accept/reject counts for both testers

Arguments:
- `graph.slg`: input slgraph file
- `0.05`: epsilon
- `8`: explicit degree bound `d`
- `1`: first seed in the range
- `20`: last seed in the range

This is useful when you want to evaluate the randomized testers across
multiple seeds instead of inspecting a single run only.

### 6) Benchmark both testers over multiple seeds

Use the benchmark script:

```bash
python3 benchmark_testers.py graph.slg 0.05 8 1 100
```

What it does:
- runs `test/slgraph_tester_classical` repeatedly for reference timing
- runs the basic tester once per seed in the range `1..100`
- runs the improved tester once per seed in the range `1..100`
- measures wall-clock time for every run
- reports total, average, minimum, and maximum runtime for each tester
- reports relative speedups between the classical, basic, and improved testers

Arguments:
- `graph.slg`: input slgraph file
- `0.05`: epsilon
- `8`: explicit degree bound `d`
- `1`: first seed in the range
- `100`: last seed in the range

This is useful when you want to compare runtime behavior rather than
only acceptance and rejection counts.

## Example Run

If you already have `bamberg-edges.txt`:

```bash
cd test
make slgraph_load_edgelist slgraph_tester_basic
cd ..

test/slgraph_load_edgelist bamberg-edges.txt bamberg.slg
test/slgraph_tester_classical bamberg.slg
test/slgraph_tester_basic bamberg.slg 0.05 8 1
test/slgraph_tester_improved bamberg.slg 0.05 8 1
python3 run_20_tests.py bamberg.slg 0.05 8 1 20
python3 benchmark_testers.py bamberg.slg 0.05 8 1 100
```

If you start from OSM:

```bash
chmod +x scripts/prepare_edgelist.sh
scripts/prepare_edgelist.sh --mode osm --input /path/to/bamberg.osm.pbf --output bamberg-edges.txt

cd test
make slgraph_load_edgelist slgraph_tester_basic
cd ..

test/slgraph_load_edgelist bamberg-edges.txt bamberg.slg
test/slgraph_tester_classical bamberg.slg
test/slgraph_tester_basic bamberg.slg 0.05 8 1
test/slgraph_tester_improved bamberg.slg 0.05 8 1
python3 run_20_tests.py bamberg.slg 0.05 8 1 20
python3 benchmark_testers.py bamberg.slg 0.05 8 1 100
```
