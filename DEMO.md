# Demo: showing the engine live

A short demo you can run by hand, in an order that tells the story: the original idea, then what it does now. Each step lists the command, what you should see, and a line you can say.

## 0. Set up (once)

Needs Linux or macOS with a C compiler and `make`.

```bash
git clone https://github.com/cadenegr/skyscrapers.git
cd skyscrapers/engine
make
```

This builds two programs: `./skyscrapers` and `./nonogram`.

## 1. Show that it's tested

```bash
make test
```

You should see `6 passed, 0 failed` (Skyscrapers) and `4 passed, 0 failed` (Nonograms).

## 2. The original 4x4 puzzle

```bash
./skyscrapers "4 3 2 1 1 2 2 2 4 3 2 1 1 2 2 2"
```

Clues are given in the order **top, bottom, left, right**. You'll see the rounds of propagation, then the solved grid with the clues around it.

> "Each row and column lists the ways it could be filled. Then rows and columns cross-check each other, round after round, until only the answer is left."

## 3. The puzzle that made the original hang

```bash
./skyscrapers "4 2 2 1 1 2 2 4 4 2 2 1 1 2 2 4"
```

It says **"more than one solution"** and shows two different grids. The original Piscine version looped forever on this one.

## 4. Impossible clues

```bash
./skyscrapers "1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1"
```

It says **"No solution"** instead of hanging or printing nonsense.

## 5. A real 9x9 from puzzle-skyscrapers.com

```bash
./skyscrapers "3 . 3 3 5 . 2 2 . 2 2 3 2 2 . . 3 3 . . 3 . 3 . 1 2 . 2 . . 4 . 3 4 . 2" "..2...... ........6 ...6.1... 34...8... .3..7..4. ..4....1. ......... .....7... ........."
```

- A `.` in the clues is a blank clue.
- The second argument holds the numbers already filled in, row by row, with `.` for an empty cell.

It's solved instantly, **by propagation alone, with no guessing**.

> "That's the idea I had in my second week of programming, solving a real 9x9."

## 6. A different puzzle, with the same engine

```bash
./nonogram "1 1 / 3 / 1 1" "3 / 1 / 3"
```

Rows first, then columns; lines are separated by `/`. It colours the grid and shows the letter **H**.

> "The core doesn't know any puzzle rules. I added Nonograms without changing it at all."

## 7. Let them try one of their own

Open any 4x4 to 9x9 puzzle on [puzzle-skyscrapers.com](https://www.puzzle-skyscrapers.com/) and type in its clues (top, bottom, left, right, with `.` for blanks), plus the grid if it has numbers already filled in. Solving a puzzle they picked is the most convincing part.

## Tips

- Use the up arrow to repeat a command and change it, instead of retyping.
- If something fails, it's almost always the clue order or a miscounted clue. The program says how many clues it got: a 6x6 needs 24, a 9x9 needs 36.
- Filled-in numbers are easy to misplace by one column. If a real puzzle comes out with "more than one solution", check them first.
