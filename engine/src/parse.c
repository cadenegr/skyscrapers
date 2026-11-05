#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "engine.h"

/*
 * Reads the clues: 4 * N numbers separated by spaces, in the same order as
 * your original program: top, bottom, left, right.
 *
 * Your original checked for exactly 31 characters, which only works for 4x4.
 * Here we just read numbers until the string ends, then work out N from how
 * many we found.
 */
bool	parse_clues(const char *input, t_puzzle *p)
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
		if (!isdigit((unsigned char)*input))
			return (fprintf(stderr, "Error: unexpected character '%c'.\n", *input), false);
		if (count == 4 * MAX_N)
			return (fprintf(stderr, "Error: too many clues (max %d).\n", 4 * MAX_N), false);
		values[count++] = (int)strtol(input, &end, 10);
		input = end;
	}
	if (count == 0 || count % 4 != 0)
		return (fprintf(stderr, "Error: need 4 x N clues, got %d.\n", count), false);
	p->n = count / 4;
	for (int i = 0; i < count; i++)
	{
		if (values[i] < 1 || values[i] > p->n)
			return (fprintf(stderr, "Error: clue %d is out of range 1..%d.\n",
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
