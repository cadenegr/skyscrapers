# Skyscrapers engine

A small constraint-solving engine in C. It started as a 4x4 Skyscrapers solver written during the 42 Berlin Piscine, and grew into a general core that now solves two kinds of puzzle: **Skyscrapers** (any size up to 9x9) and **Nonograms**.

The idea at its heart comes from that first solver: list every way each row and column could be filled, then let rows and columns cross-check each other until only the answer is left.

## Quick start

```bash
git clone https://github.com/cadenegr/skyscrapers.git
cd skyscrapers/engine
make          # builds ./skyscrapers and ./nonogram
make test     # runs the known puzzles
```

Requires a C99 compiler and `make`. No other dependencies. To show it to someone step by step, follow [DEMO.md](DEMO.md).

## Skyscrapers

Fill an N×N grid with buildings of height 1 to N, each height once per row and column. A clue outside the grid says how many buildings you see from that side: taller buildings hide shorter ones behind them.

```bash
./skyscrapers "top  bottom  left  right"  ["grid"]
```

- Clues are given in the order top, bottom (both left to right), then left, right (both top to bottom). Write a blank clue as `.`.
- The optional second argument gives numbers already filled in, row by row, with `.` for an empty cell.

A real 9x9 from [puzzle-skyscrapers.com](https://www.puzzle-skyscrapers.com/), with blank clues and 15 filled-in numbers:

```bash
./skyscrapers "3 . 3 3 5 . 2 2 . 2 2 3 2 2 . . 3 3 . . 3 . 3 . 1 2 . 2 . . 4 . 3 4 . 2" \
              "..2...... ........6 ...6.1... 34...8... .3..7..4. ..4....1. ......... .....7... ........."
```
```
Solved by propagation alone.

      3 . 3 3 5 . 2 2 .
  . |1|9|2|7|5|3|4|6|8| 2
  . |8|7|1|3|4|5|9|2|6| .
  3 |4|2|8|6|3|1|7|9|5| .
  . |3|4|9|2|6|8|5|7|1| 4
  3 |6|3|5|8|7|2|1|4|9| .
  . |2|5|4|9|8|6|3|1|7| 3
  1 |9|6|7|1|2|4|8|5|3| 4
  2 |5|1|3|4|9|7|6|8|2| .
  . |7|8|6|5|1|9|2|3|4| 2
      2 2 3 2 2 . . 3 3
```

If a puzzle has more than one solution, the program says so and prints two of them. If the clues contradict each other, it says there is no solution.

## Nonograms

Colour in a grid to reveal a picture. The numbers for each row and column give the blocks of filled squares in that line, in order, with at least one empty square between blocks.

```bash
./nonogram "row clues" "column clues"
./nonogram "1 1 / 3 / 1 1" "3 / 1 / 3"      # the letter H
```

Lines are separated by `/`; `0` is an empty line. The tests include an 11x18 city skyline that the engine rebuilds from its numbers alone.

## How it works

The engine sees a puzzle as **cells**, each holding a small value (0 to 31), and **groups** of cells, such as a row or a column. Each group has a list of **candidates**: every way that group could be filled.

1. **Narrow the cells.** Each puzzle's own rules narrow what each cell can hold before any lists are built (in Skyscrapers, a clue of 1 means the tallest building stands next to it). Numbers already filled in narrow their cell to one value.
2. **Build each group's candidates,** checking each cell as soon as it is decided, so a bad start is dropped together with every way of finishing it. Each finished group narrows the cells it covers, so the groups built after it save fewer candidates.
3. **Propagate.** For every cell, keep only the values that all its groups' remaining candidates agree on, and rule out candidates that use anything else. Repeat until a round changes nothing.
4. **Guess when stuck.** Pick the group with the fewest candidates left, try one, propagate, and undo if it leads to a contradiction. The search stops after two solutions, which is enough to tell whether a puzzle is unique.

The core knows nothing about Skyscrapers or Nonograms. A puzzle plug-in only describes its cells, its groups, and each group's candidates; propagation and guessing are shared.

```
engine/
├── core/                    the reusable engine (libengine.a)
│   ├── include/engine.h     the whole interface
│   └── src/                 engine.c, propagate.c, search.c
├── puzzles/
│   ├── skyscrapers/         clues, clue limits, visibility, printing
│   └── nonogram/            block clues, block placement, printing
└── tests/                   known puzzles, run by `make test`
```

## Writing a plug-in

```c
#include "engine.h"

t_engine    e;
t_solutions s;

engine_init(&e, number_of_cells, BIT(0) | BIT(1));  // every cell starts as 0 or 1
int g = engine_add_group(&e, cells, len);           // a group: which cells, in order
engine_add_candidate(&e, g, values);                // one way to fill it, once per candidate
engine_finish_group(&e, g);                         // done: narrows the group's cells
engine_solve(&e, &s, 2);                            // s.found: 0 none, 1 unique, 2 several
```

A cell can belong to any number of groups, so Sudoku boxes or crossword slots fit the same model. See [puzzles/nonogram/main.c](engine/puzzles/nonogram/main.c) for a complete plug-in in one file.

## Limits

- Candidates are stored in full, so a group with millions of possible fillings uses a lot of memory. A 9x9 Skyscrapers with no clues at all saves 6.5 million candidates (about 150 MB, under a second).
- Guessing tries whole candidates. That works well for designed puzzles, but a random 20x20 Nonogram took 77 seconds and 175,328 guesses.
- Groups have at most 64 cells, and values go from 0 to 31.

## History

The original Piscine solver is in [skyscrapers/](skyscrapers/) and tagged `v1`. It solves every 4x4 puzzle that has a single solution, but never finishes on the 66 clue sets that have more than one, because its loop has no check for a round that changes nothing. The engine in [engine/](engine/) is the same idea rebuilt, generalized and tested.

## License

[MIT](LICENSE) © 2026 Carlos Denegri (cadenegr)
