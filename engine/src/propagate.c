#include <stdio.h>
#include "engine.h"

/* Which cell of the grid is position `pos` of line `l`? */
static void	cell_of(const t_puzzle *p, int l, int pos, int *r, int *c)
{
	if (l < p->n)
	{
		*r = l;
		*c = pos;
	}
	else
	{
		*r = pos;
		*c = l - p->n;
	}
}

int	total_alive(const t_puzzle *p)
{
	int	total = 0;

	for (int l = 0; l < 2 * p->n; l++)
		total += p->lines[l].alive_count;
	return (total);
}

/*
 * Part 1 of a round: for every cell, which heights do the row's candidates
 * put there, and which do the column's? Keep only heights both agree on.
 *
 * This is your check_row / check_column, turned around. Your version asked,
 * for each row candidate, "does some column candidate have the same digit
 * here?", searching the column's list each time. Here each line goes through
 * its own list once and records the heights it uses at each position. After
 * that, "does the column allow this?" is a single AND.
 */
static void	update_allowed(t_puzzle *p)
{
	uint16_t	by_row[MAX_N][MAX_N] = {{0}};
	uint16_t	by_col[MAX_N][MAX_N] = {{0}};
	int			n = p->n;

	for (int l = 0; l < 2 * n; l++)
	{
		t_line	*line = &p->lines[l];

		for (int k = 0; k < line->count; k++)
		{
			if (!line->alive[k])
				continue ;
			for (int pos = 0; pos < n; pos++)
			{
				int	h = line->cells[k * n + pos];
				int	r;
				int	c;

				cell_of(p, l, pos, &r, &c);
				if (l < n)
					by_row[r][c] |= BIT(h);
				else
					by_col[r][c] |= BIT(h);
			}
		}
	}
	for (int r = 0; r < n; r++)
		for (int c = 0; c < n; c++)
			p->allowed[r][c] &= by_row[r][c] & by_col[r][c];
}

/*
 * Part 2 of a round: rule out every candidate that puts a height somewhere
 * it is no longer allowed. Returns how many were ruled out.
 */
static int	remove_candidates(t_puzzle *p)
{
	int	removed = 0;
	int	n = p->n;

	for (int l = 0; l < 2 * n; l++)
	{
		t_line	*line = &p->lines[l];

		for (int k = 0; k < line->count; k++)
		{
			if (!line->alive[k])
				continue ;
			for (int pos = 0; pos < n; pos++)
			{
				int	r;
				int	c;

				cell_of(p, l, pos, &r, &c);
				if (!HAS(p->allowed[r][c], line->cells[k * n + pos]))
				{
					line->alive[k] = false;
					line->alive_count--;
					removed++;
					break ;
				}
			}
		}
	}
	return (removed);
}

/*
 * Runs rounds until nothing changes: the loop in your build_result().
 *
 * The difference from your version: when a round removes nothing, we stop
 * and say whether the puzzle is solved or stuck. Your loop kept going in
 * that case, which is why it hung on puzzles with several solutions.
 */
t_result	propagate(t_puzzle *p, bool verbose)
{
	int	round = 0;

	while (1)
	{
		int	removed;

		update_allowed(p);
		removed = remove_candidates(p);
		round++;
		if (verbose)
			printf("  round %d: removed %d, %d candidates left\n",
				round, removed, total_alive(p));
		for (int l = 0; l < 2 * p->n; l++)
			if (p->lines[l].alive_count == 0)
				return (IMPOSSIBLE);
		if (removed == 0)
			break ;
	}
	for (int l = 0; l < 2 * p->n; l++)
		if (p->lines[l].alive_count != 1)
			return (STUCK);
	return (SOLVED);
}
