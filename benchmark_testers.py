#!/usr/bin/env python3
import subprocess
import sys
import time
from statistics import mean


# Usage:
#   python3 benchmark_testers.py [graph_path] [epsilon] [d] [seed_start] [seed_end]
#
# Example:
#   python3 benchmark_testers.py bamberg.slg 0.05 8 1 100


def run_one(cmd):
    t0 = time.perf_counter()
    cp = subprocess.run(cmd, capture_output=True, text=True, check=True)
    dt = time.perf_counter() - t0
    lines = [ln.strip() for ln in cp.stdout.splitlines() if ln.strip()]
    final = lines[-1] if lines else ""
    return dt, final


def bench(exe, graph, eps, degree, seed_start, seed_end, use_seed):
    times = []
    accepts = 0
    rejects = 0
    sample_lines = []

    for seed in range(seed_start, seed_end + 1):
        cmd = [exe, graph, eps, degree, str(seed)] if use_seed else [exe, graph]
        dt, final = run_one(cmd)
        times.append(dt)
        if final.startswith("ACCEPT"):
            accepts += 1
        elif final.startswith("REJECT"):
            rejects += 1
        if len(sample_lines) < 5:
            sample_lines.append((seed, final))

    return {
        "runs": len(times),
        "total": sum(times),
        "avg": mean(times),
        "min": min(times),
        "max": max(times),
        "accepts": accepts,
        "rejects": rejects,
        "samples": sample_lines,
    }


def print_result(name, result):
    print(
        f"{name} total={result['total']:.6f}s avg={result['avg']:.6f}s "
        f"min={result['min']:.6f}s max={result['max']:.6f}s "
        f"accepts={result['accepts']} rejects={result['rejects']}"
    )
    for seed, final in result["samples"]:
        print(f"  seed={seed}: {final}")


def main():
    graph = "bamberg.slg"
    eps = "0.05"
    degree = "8"
    seed_start = 1
    seed_end = 100

    if len(sys.argv) >= 2:
        graph = sys.argv[1]
    if len(sys.argv) >= 3:
        eps = sys.argv[2]
    if len(sys.argv) >= 4:
        degree = sys.argv[3]
    if len(sys.argv) >= 5:
        seed_start = int(sys.argv[4])
    if len(sys.argv) >= 6:
        seed_end = int(sys.argv[5])

    print(f"GRAPH={graph} eps={eps} d={degree} seeds={seed_start}..{seed_end}\n")

    classical = bench("test/slgraph_tester_classical", graph, eps, degree, seed_start, seed_end, False)
    basic = bench("test/slgraph_tester_basic", graph, eps, degree, seed_start, seed_end, True)
    improved = bench("test/slgraph_tester_improved", graph, eps, degree, seed_start, seed_end, True)

    print("CLASSICAL")
    print_result("classical", classical)
    print()

    print("BASIC")
    print_result("basic", basic)
    print()

    print("IMPROVED")
    print_result("improved", improved)
    print()

    if improved["avg"] > 0:
        speedup = basic["avg"] / improved["avg"]
        print(f"speedup basic/improved={speedup:.3f}x")
    if classical["avg"] > 0:
        print(f"speedup classical/basic={classical['avg'] / basic['avg']:.3f}x")
        print(f"speedup classical/improved={classical['avg'] / improved['avg']:.3f}x")


if __name__ == "__main__":
    main()
