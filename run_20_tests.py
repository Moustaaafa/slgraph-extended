#!/usr/bin/env python3
import subprocess
import sys
from collections import Counter


# Usage:
#   python3 run_20_tests.py [graph_path] [epsilon] [d] [seed_start] [seed_end]
#
# Example:
#   python3 run_20_tests.py bamberg.slg 0.1 9 1 20


def run_one(cmd):
    out = subprocess.check_output(cmd, text=True)
    lines = [ln.strip() for ln in out.strip().splitlines() if ln.strip()]
    return lines[0], lines[-1]  # stats, final line


def main():
    graph = "bamberg.slg"
    eps = "0.1"
    d = "9"
    seed_start = 1
    seed_end = 20

    if len(sys.argv) >= 2:
        graph = sys.argv[1]
    if len(sys.argv) >= 3:
        eps = sys.argv[2]
    if len(sys.argv) >= 4:
        d = sys.argv[3]
    if len(sys.argv) >= 5:
        seed_start = int(sys.argv[4])
    if len(sys.argv) >= 6:
        seed_end = int(sys.argv[5])

    basic_counts = Counter()
    improved_counts = Counter()

    print(f"GRAPH={graph}  eps={eps}  d={d}  seeds={seed_start}..{seed_end}\n")

    print("BASIC TESTER")
    for seed in range(seed_start, seed_end + 1):
        _stats, final = run_one(["test/slgraph_tester_basic", graph, eps, d, str(seed)])
        status = "ACCEPT" if final.startswith("ACCEPT") else "REJECT"
        basic_counts[status] += 1
        print(f"seed={seed:2d}  {status:6s}  {final}")
    print(f"BASIC TOTAL: ACCEPT={basic_counts['ACCEPT']}, REJECT={basic_counts['REJECT']}\n")

    print("IMPROVED TESTER")
    for seed in range(seed_start, seed_end + 1):
        _stats, final = run_one(["test/slgraph_tester_improved", graph, eps, d, str(seed)])
        status = "ACCEPT" if final.startswith("ACCEPT") else "REJECT"
        improved_counts[status] += 1
        print(f"seed={seed:2d}  {status:6s}  {final}")
    print(f"IMPROVED TOTAL: ACCEPT={improved_counts['ACCEPT']}, REJECT={improved_counts['REJECT']}")


if __name__ == "__main__":
    main()
