#!/bin/sh
# Runs the nonogram program on known puzzles and checks the answers.
# Usage: tests/run_nonogram_tests.sh [path/to/nonogram]   (or: make test)

BIN="${1:-./nonogram}"
pass=0
fail=0

# check NAME EXPECTED ROWS COLUMNS
#   EXPECTED is "unique:<picture row by row, # and .>", "several" or "none".
check() {
	name="$1"; expected="$2"; shift 2
	out=$("$BIN" "$@" 2>&1)
	case "$out" in
		*"No solution"*)   got="none" ;;
		*"more than one"*) got="several" ;;
		*"Solved"*)        got="unique:$(printf '%s\n' "$out" | grep -E '[#.]$' | grep -E '^[ 0-9]+( +[#.])+$' \
		                       | sed -E 's/^ *[0-9 ]*[0-9] +//; s/ //g' | tr -d '\n')" ;;
		*)                 got="error" ;;
	esac
	if [ "$got" = "$expected" ]; then
		pass=$((pass + 1)); echo "ok    $name"
	else
		fail=$((fail + 1)); echo "FAIL  $name"; echo "      expected: $expected"; echo "      got:      $got"
	fi
}

check "3x3 letter H" "unique:#.#####.#" "1 1 / 3 / 1 1" "3 / 1 / 3"

# Two diagonals both fit: a puzzle with two solutions.
check "2x2 with two solutions" "several" "1 / 1" "1 / 1"

# A row of 3 in a 2-wide grid can't fit.
check "impossible" "none" "3" "1 / 1"

# An 11x18 skyline, rebuilt from its numbers alone.
check "skyline 11x18" "unique:.......#................###...............###......#....##..#.#.....###...##..###.....#.#...##..#.#..##.###.#.##..###..##.#.#.#.##.##.##.##.###.####.##.##.###########.#####.#.#.#.###################" \
	"1 / 3 / 3 1 / 2 1 1 3 / 2 3 1 1 / 2 1 1 2 3 / 1 2 3 2 1 1 / 1 2 2 2 2 3 / 4 2 2 7 / 4 5 1 1 1 1 / 18" \
	"5 / 3 / 8 / 8 / 1 / 4 / 10 / 3 1 1 2 / 10 / 4 / 1 / 6 / 4 1 / 3 / 6 1 / 2 1 4 / 6 1 / 3"

echo "$pass passed, $fail failed"
[ "$fail" -eq 0 ]
