#ifndef ENGINE_H
#define ENGINE_H

#include <stdbool.h>
#include <stdint.h>

/* Largest grid we accept. Clues are single digits, so 9 is the natural limit. */
#define MAX_N 9

/*
 * One line of the grid (a row or a column) and every way it could be filled.
 *
 * This is the same idea as your original `possibilities` string, but each
 * line gets its own list instead of sharing one string with separators.
 * Candidate k occupies cells[k * n] .. cells[k * n + n - 1].
 */
typedef struct s_line
{
	int		count;		/* how many candidates were generated */
	int		capacity;	/* how many fit in `cells` before we must grow it */
	uint8_t	*cells;		/* the candidates, n heights each */
	bool	*alive;		/* alive[k] is false once candidate k is ruled out */
	int		alive_count;
}	t_line;

/*
 * The whole puzzle.
 * Lines 0 .. n-1 are the rows (top to bottom),
 * lines n .. 2n-1 are the columns (left to right).
 */
typedef struct s_puzzle
{
	int		n;
	int		top[MAX_N];		/* seen from above each column */
	int		bottom[MAX_N];	/* seen from below each column */
	int		left[MAX_N];	/* seen from the left of each row */
	int		right[MAX_N];	/* seen from the right of each row */
	/*
	 * allowed[r][c]: which heights may still go in cell (r, c).
	 * Bit h is set if height h is allowed, so with n = 4 a fresh cell is
	 * 0b11110 (heights 1, 2, 3, 4). One small number per cell instead of
	 * a list makes "is h allowed here?" a single AND.
	 */
	uint16_t	allowed[MAX_N][MAX_N];
	t_line	lines[2 * MAX_N];
}	t_puzzle;

/* The bit for height h, and the test "is h in this set?" */
#define BIT(h)			((uint16_t)(1u << (h)))
#define HAS(set, h)		(((set) & BIT(h)) != 0)

/* parse.c */
bool	parse_clues(const char *input, t_puzzle *p);

/* What a round of propagation can end with. */
typedef enum e_result
{
	SOLVED,			/* every line has exactly one candidate left */
	STUCK,			/* nothing changes any more, but some lines still have several */
	IMPOSSIBLE		/* some line ran out of candidates: no solution */
}	t_result;

/* propagate.c */
t_result	propagate(t_puzzle *p, bool verbose);
int			total_alive(const t_puzzle *p);

/* search.c: guessing, for when propagation gets stuck */
#define MAX_SOLUTIONS 2		/* finding 2 is enough to prove "not unique" */

typedef struct s_search
{
	int		found;									/* solutions found so far */
	uint8_t	solution[MAX_SOLUTIONS][MAX_N][MAX_N];	/* the solved grids */
	long	guesses;								/* how many candidates we tried */
}	t_search;

bool	solve(t_puzzle *p, t_search *s);

/* clues.c */
void	apply_clue_limits(t_puzzle *p);

/* lines.c */
int		visible_count(const uint8_t *heights, int n, bool reversed);
bool	build_lines(t_puzzle *p);
void	free_lines(t_puzzle *p);

#endif
