#include "engine.h"

/*
 * Your idea: before building any line, use ALL four clues to limit what each
 * cell can hold, so a row also respects the top/bottom clues of the columns
 * it crosses (and a column the left/right clues of the rows).
 *
 * The rule, for a clue k and a cell d steps in from that edge (d = 0 is the
 * cell next to the clue):
 *
 *   - k == 1: the tallest building (n) must be first, so cell 0 holds n
 *             and no other cell of that line can.
 *   - k >= 2: cell d holds at most  n - k + 1 + d.
 *
 * Why: if a building of height h stands d steps in, you can see at most the
 * d buildings in front of it, itself, and the (n - h) buildings taller than
 * it behind it. So k <= d + 1 + (n - h), which gives h <= n - k + 1 + d.
 *
 * Example on 9x9 with top clue 7: the top cell is at most 3, the next at
 * most 4, then 5, and so on.
 */
static void	limit_cell(t_puzzle *p, int r, int c, int clue, int d)
{
	uint16_t	keep = 0;
	int			max_h;

	if (clue == 0)
		return ;	/* blank clue: tells us nothing */
	if (clue == 1)
	{
		if (d == 0)
			p->allowed[r][c] &= BIT(p->n);
		else
			p->allowed[r][c] &= (uint16_t)~BIT(p->n);
		return ;
	}
	max_h = p->n - clue + 1 + d;
	for (int h = 1; h <= p->n && h <= max_h; h++)
		keep |= BIT(h);
	p->allowed[r][c] &= keep;
}

void	apply_clue_limits(t_puzzle *p)
{
	int	n = p->n;

	for (int r = 0; r < n; r++)
		for (int c = 0; c < n; c++)
			p->allowed[r][c] = (uint16_t)(BIT(n + 1) - 2); /* bits 1..n */
	for (int i = 0; i < n; i++)
	{
		for (int d = 0; d < n; d++)
		{
			limit_cell(p, d, i, p->top[i], d);				/* column i, from the top */
			limit_cell(p, n - 1 - d, i, p->bottom[i], d);	/* column i, from the bottom */
			limit_cell(p, i, d, p->left[i], d);				/* row i, from the left */
			limit_cell(p, i, n - 1 - d, p->right[i], d);	/* row i, from the right */
		}
	}
}
