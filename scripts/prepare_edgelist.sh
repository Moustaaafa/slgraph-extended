#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
prepare_edgelist.sh - build "u v" edge list text for slgraph_load_edgelist

Usage:
  scripts/prepare_edgelist.sh --mode osm --input map.osm.pbf --output edges.txt
  scripts/prepare_edgelist.sh --mode osm --input map.osm --output edges.txt
  scripts/prepare_edgelist.sh --mode table --input graph.csv --output edges.txt [options]

Modes:
  osm
    Input is OpenStreetMap data (.osm or .osm.pbf). Road ways are filtered
    by tag "highway", and consecutive node refs are emitted as edges.

  table
    Input is a generic network edge table (txt/csv/tsv/etc). Two columns are
    selected as source/target and normalized into "u v" lines.

Options:
  --mode MODE         Required: osm | table
  --input PATH        Required: input file path
  --output PATH       Required: output edge-list file path

  --delimiter STR     table mode only. Default: auto-detect (comma -> CSV,
                      otherwise whitespace split)
  --src-col N         table mode only. 1-based source column index (default 1)
  --dst-col N         table mode only. 1-based destination column index (default 2)
  --skip-header       table mode only. Skip first line
  --allow-nonnumeric  table mode only. Keep non-numeric node IDs (default off)
  --undirected        Emit reverse edge "v u" for each edge and deduplicate
  --dedup             Deduplicate duplicate edges in output
  --help              Show this help

Dependencies:
  osm mode: osmium, jq
  table mode: awk
EOF
}

die() {
  echo "Error: $*" >&2
  exit 1
}

require_cmd() {
  command -v "$1" >/dev/null 2>&1 || die "Missing required command: $1"
}

MODE=""
INPUT=""
OUTPUT=""
DELIM=""
SRC_COL=1
DST_COL=2
SKIP_HEADER=0
ALLOW_NONNUMERIC=0
UNDIRECTED=0
DEDUP=0

while [[ $# -gt 0 ]]; do
  case "$1" in
    --mode)
      [[ $# -ge 2 ]] || die "--mode requires a value"
      MODE="$2"
      shift 2
      ;;
    --input)
      [[ $# -ge 2 ]] || die "--input requires a value"
      INPUT="$2"
      shift 2
      ;;
    --output)
      [[ $# -ge 2 ]] || die "--output requires a value"
      OUTPUT="$2"
      shift 2
      ;;
    --delimiter)
      [[ $# -ge 2 ]] || die "--delimiter requires a value"
      DELIM="$2"
      shift 2
      ;;
    --src-col)
      [[ $# -ge 2 ]] || die "--src-col requires a value"
      SRC_COL="$2"
      shift 2
      ;;
    --dst-col)
      [[ $# -ge 2 ]] || die "--dst-col requires a value"
      DST_COL="$2"
      shift 2
      ;;
    --skip-header)
      SKIP_HEADER=1
      shift
      ;;
    --allow-nonnumeric)
      ALLOW_NONNUMERIC=1
      shift
      ;;
    --undirected)
      UNDIRECTED=1
      shift
      ;;
    --dedup)
      DEDUP=1
      shift
      ;;
    --help|-h)
      usage
      exit 0
      ;;
    *)
      die "Unknown argument: $1"
      ;;
  esac
done

[[ -n "$MODE" ]] || die "--mode is required"
[[ -n "$INPUT" ]] || die "--input is required"
[[ -n "$OUTPUT" ]] || die "--output is required"
[[ -f "$INPUT" ]] || die "Input file not found: $INPUT"
[[ "$SRC_COL" =~ ^[0-9]+$ ]] || die "--src-col must be a positive integer"
[[ "$DST_COL" =~ ^[0-9]+$ ]] || die "--dst-col must be a positive integer"
(( SRC_COL >= 1 )) || die "--src-col must be >= 1"
(( DST_COL >= 1 )) || die "--dst-col must be >= 1"

tmp_out="$(mktemp /tmp/edgelist_raw_XXXXXX.txt)"
trap 'rm -f "$tmp_out"' EXIT

if [[ "$MODE" == "osm" ]]; then
  require_cmd osmium
  require_cmd jq

  tmp_pbf="$(mktemp /tmp/osm_highways_XXXXXX.osm.pbf)"
  tmp_json="$(mktemp /tmp/osm_highways_XXXXXX.jsonseq)"
  trap 'rm -f "$tmp_out" "$tmp_pbf" "$tmp_json"' EXIT

  # Keep road ways and export their node-reference sequences.
  osmium tags-filter "$INPUT" w/highway -o "$tmp_pbf" --overwrite
  osmium export "$tmp_pbf" -f jsonseq -o "$tmp_json" --overwrite

  jq -r '
    select(.type == "Feature")
    | .properties.nodes as $n
    | if ($n | length) > 1 then
        range(0; ($n | length) - 1) as $i
        | "\($n[$i]) \($n[$i + 1])"
      else empty end
  ' "$tmp_json" > "$tmp_out"

elif [[ "$MODE" == "table" ]]; then
  require_cmd awk

  awk -v src="$SRC_COL" -v dst="$DST_COL" \
      -v delim="$DELIM" -v skip_header="$SKIP_HEADER" \
      -v allow_nonnumeric="$ALLOW_NONNUMERIC" '
    BEGIN {
      if (delim != "") {
        FS = delim;
      } else {
        FS = "[[:space:]]+";
      }
    }
    NR == 1 && skip_header == 1 { next }
    /^[[:space:]]*$/ { next }
    /^[[:space:]]*#/ { next }
    {
      if (delim == "" && index($0, ",") > 0) {
        split($0, f, ",");
        u = f[src];
        v = f[dst];
      } else {
        if (NF < src || NF < dst) next;
        u = $src;
        v = $dst;
      }

      gsub(/^[[:space:]]+|[[:space:]]+$/, "", u);
      gsub(/^[[:space:]]+|[[:space:]]+$/, "", v);
      if (u == "" || v == "") next;

      if (!allow_nonnumeric) {
        if (u !~ /^[0-9]+$/ || v !~ /^[0-9]+$/) next;
      }

      print u " " v;
    }
  ' "$INPUT" > "$tmp_out"

else
  die "Unsupported --mode: $MODE (expected osm or table)"
fi

if (( UNDIRECTED == 1 )); then
  awk '{ print $1 " " $2; print $2 " " $1 }' "$tmp_out" > "${tmp_out}.undir"
  mv "${tmp_out}.undir" "$tmp_out"
  DEDUP=1
fi

if (( DEDUP == 1 )); then
  awk '!seen[$0]++' "$tmp_out" > "$OUTPUT"
else
  cp "$tmp_out" "$OUTPUT"
fi

echo "Wrote edge list: $OUTPUT"
