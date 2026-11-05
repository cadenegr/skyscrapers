#ifndef SKYSCRAPERS_H
#define SKYSCRAPERS_H

#include "engine.h"

/*
 * The Skyscrapers plug-in: everything about this puzzle that the engine
 * doesn't know. Cell (r, c) of the grid is engine cell r * n + c, and its
 * value is the building height, 1 to n.
 */

/* Largest grid we accept. Clues are single digits, so 9 is the natural limit. */
#define MAX_N 9

typedef struct s_sky
{
	int		n;
	int		top[MAX_N];		/* seen from above each column, 0 = blank */
	int		bottom[MAX_N];	/* seen from below each column */
	int		left[MAX_N];	/* seen from the left of each row */
	int		right[MAX_N];	/* seen from the right of each row */
}	t_sky;

/* parse.c */
bool	parse_clues(const char *input, t_sky *sky);
bool	apply_givens(const char *grid, const t_sky *sky, t_engine *e);

/* clues.c */
void	apply_clue_limits(const t_sky *sky, t_engine *e);

/* lines.c */
int		visible_count(const uint8_t *heights, int n, bool reversed);
bool	build_lines(const t_sky *sky, t_engine *e);

#endif
