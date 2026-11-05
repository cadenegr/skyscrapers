#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "skyscrapers.h"

/*
 * Reads the clues: 4 * N numbers separated by spaces, in the same order as
 * your original program: top, bottom, left, right.
 *
 * A blank clue (no number on that side in the puzzle) is written as 0,
 * '.' or '-', and stored as 0.
 *
 * Your original checked for exactly 31 characters, which only works for 4x4.
 * Here we just read numbers until the string ends, then work out N from how
 * many we found.
 */
bool	parse_clues(const char *input, t_sky *p)
{
	int		values[4 * MAX_N];
	int		count = 0;
	char	*end;

	while (*input)
	{
		if (isspace((unsigned char)*input))
		{
			input++;
			continue ;
		}
		if (count == 4 * MAX_N)
			return (fprintf(stderr, "Error: too many clues (max %d).\n", 4 * MAX_N), false);
		if (*input == '.' || *input == '-')
		{
			values[count++] = 0;	/* blank clue */
			input++;
			continue ;
		}
		if (!isdigit((unsigned char)*input))
			return (fprintf(stderr, "Error: unexpected character '%c'.\n", *input), false);
		values[count++] = (int)strtol(input, &end, 10);
		input = end;
	}
	if (count == 0 || count % 4 != 0)
		return (fprintf(stderr, "Error: need 4 x N clues, got %d.\n", count), false);
	p->n = count / 4;
	for (int i = 0; i < count; i++)
	{
		if (values[i] < 0 || values[i] > p->n)
			return (fprintf(stderr, "Error: clue %d is out of range 0..%d.\n",
					values[i], p->n), false);
	}
	for (int i = 0; i < p->n; i++)
	{
		p->top[i] = values[i];
		p->bottom[i] = values[p->n + i];
		p->left[i] = values[2 * p->n + i];
		p->right[i] = values[3 * p->n + i];
	}
	return (true);
}

/*
 * Applies the numbers already filled in inside the grid (optional second
 * argument). The grid is written row by row, N characters per row, with '.'
 * or '0' for an empty cell. Spaces and line breaks are ignored, so both
 * "...... .....4 ..." and one long string work.
 *
 * A given number simply limits its cell to that one height. Must be called
 * after engine_init(), which starts every cell from "all heights".
 */
bool	apply_givens(const char *grid, const t_sky *p, t_engine *e)
{
	int	cell = 0;

	for (; *grid; grid++)
	{
		if (isspace((unsigned char)*grid))
			continue ;
		if (cell == p->n * p->n)
			return (fprintf(stderr, "Error: grid has more than %d cells.\n",
					p->n * p->n), false);
		if (*grid != '.' && *grid != '0')
		{
			int	h = *grid - '0';

			if (h < 1 || h > p->n)
				return (fprintf(stderr, "Error: '%c' in the grid is not a height 1..%d.\n",
						*grid, p->n), false);
			e->domain[cell] &= BIT(h);
		}
		cell++;
	}
	if (cell != p->n * p->n)
		return (fprintf(stderr, "Error: grid has %d cells, expected %d.\n",
				cell, p->n * p->n), false);
	return (true);
}
