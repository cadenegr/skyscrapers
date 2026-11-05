#!/bin/sh
# Runs the skyscrapers program on known puzzles and checks the answers.
# Usage: tests/run_tests.sh [path/to/skyscrapers]   (run from engine/, or: make test)

BIN="${1:-./skyscrapers}"
pass=0
fail=0

# check NAME EXPECTED CLUES [GRID]
#   EXPECTED is "unique:<solution digits row by row>", "several" or "none".
check() {
	name="$1"; expected="$2"; shift 2
	out=$("$BIN" "$@" 2>&1)
	digits=$(printf '%s\n' "$out" | grep '|.*|' | head -n 50 | sed 's/^[^|]*|//; s/| *[^|]*$//; s/|//g' | tr -d '\n')
	case "$out" in
		*"No solution"*)        got="none" ;;
		*"more than one"*)      got="several" ;;
		*"Solved"*)             got="unique:$digits" ;;
		*)                      got="error" ;;
	esac
	if [ "$got" = "$expected" ]; then
		pass=$((pass + 1)); echo "ok    $name"
	else
		fail=$((fail + 1)); echo "FAIL  $name"; echo "      expected: $expected"; echo "      got:      $got"
	fi
}

# The example from the original README (4x4, all clues).
check "readme 4x4" "unique:1234234134124123" \
	"4 3 2 1 1 2 2 2 4 3 2 1 1 2 2 2"

# Made the original v1 loop forever: two solutions.
check "4x4 with two solutions" "several" \
	"4 2 2 1 1 2 2 4 4 2 2 1 1 2 2 4"

# Clues that contradict each other.
check "impossible 4x4" "none" \
	"1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1"

# Real puzzle from puzzle-skyscrapers.com: 6x6, blank clues, one filled-in number.
check "real 6x6" "unique:163245254316516423321654432561645132" \
	"4 . 3 4 . . . . . . . 3 2 3 . 2 3 . . . 3 3 . 4" \
	"...... ...... ...... .....4 ...... ......"

# Real puzzle from puzzle-skyscrapers.com: 9x9, blank clues, 15 filled-in numbers.
check "real 9x9" "unique:192753468871345926428631795349268571635872149254986317967124853513497682786519234" \
	"3 . 3 3 5 . 2 2 . 2 2 3 2 2 . . 3 3 . . 3 . 3 . 1 2 . 2 . . 4 . 3 4 . 2" \
	"..2...... ........6 ...6.1... 34...8... .3..7..4. ..4....1. ......... .....7... ........."

# The same 9x9 with two filled-in numbers one column off: several solutions.
check "real 9x9, misread numbers" "several" \
	"3 . 3 3 5 . 2 2 . 2 2 3 2 2 . . 3 3 . . 3 . 3 . 1 2 . 2 . . 4 . 3 4 . 2" \
	"..2...... ........6 ...6.1... 34...8... .3..7.4.. ..4...1.. ......... .....7... ........."

echo "$pass passed, $fail failed"
[ "$fail" -eq 0 ]
