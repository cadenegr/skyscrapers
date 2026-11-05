#include <stdlib.h>
#include <string.h>
#include "engine.h"

/*
 * Step 3: guessing.
 *
 * When propagation is STUCK, some line still has several candidates. We pick
 * one, pretend it is the answer, and propagate again:
 *   - IMPOSSIBLE: that candidate was wrong, undo and try the next one;
 *   - SOLVED:     we found a solution;
 *   - STUCK:      still not sure, so guess again one level deeper.
 *
 * "Undo" means putting the puzzle back exactly as it was before the guess.
 * That is what the snapshot is for.
 */

/* Everything propagation changes: the allowed heights and the alive flags. */
typedef struct s_snapshot
{
	uint16_t	allowed[MAX_N][MAX_N];
	int			alive_count[2 * MAX_N];
	bool		*alive[2 * MAX_N];
}	t_snapshot;

static bool	save(const t_puzzle *p, t_snapshot *s)
{
	memcpy(s->allowed, p->allowed, sizeof(s->allowed));
	for (int l = 0; l < 2 * p->n; l++)
	{
		const t_line	*line = &p->lines[l];

		s->alive_count[l] = line->alive_count;
		s->alive[l] = malloc(line->count ? (size_t)line->count : 1);
		if (!s->alive[l])
		{
			while (--l >= 0)
				free(s->alive[l]);
			return (false);
		}
		memcpy(s->alive[l], line->alive, (size_t)line->count);
	}
	return (true);
}

static void	restore(t_puzzle *p, const t_snapshot *s)
{
	memcpy(p->allowed, s->allowed, sizeof(p->allowed));
	for (int l = 0; l < 2 * p->n; l++)
	{
		p->lines[l].alive_count = s->alive_count[l];
		memcpy(p->lines[l].alive, s->alive[l], (size_t)p->lines[l].count);
	}
}

static void	drop(const t_puzzle *p, t_snapshot *s)
{
	for (int l = 0; l < 2 * p->n; l++)
		free(s->alive[l]);
}

/*
 * Which line to guess on: the one with the fewest candidates left (but more
 * than one). With 2 candidates, one of our 2 guesses must be right; with
 * 40,000 we would waste a lot of time on wrong ones.
 */
static int	pick_line(const t_puzzle *p)
{
	int	best = -1;

	for (int l = 0; l < 2 * p->n; l++)
	{
		int	count = p->lines[l].alive_count;

		if (count > 1 && (best < 0 || count < p->lines[best].alive_count))
			best = l;
	}
	return (best);
}

/* Copies the solved grid (one height left in each cell) into s. */
static void	record(const t_puzzle *p, t_search *s)
{
	for (int r = 0; r < p->n; r++)
		for (int c = 0; c < p->n; c++)
			for (int h = 1; h <= p->n; h++)
				if (p->allowed[r][c] == BIT(h))
					s->solution[s->found][r][c] = (uint8_t)h;
	s->found++;
}

/* Returns false only if we ran out of memory. */
static bool	search(t_puzzle *p, t_search *s)
{
	t_result	result = propagate(p, false);
	t_snapshot	snap;
	t_line		*line;
	int			l;

	if (result == IMPOSSIBLE)
		return (true);
	if (result == SOLVED)
	{
		record(p, s);
		return (true);
	}
	l = pick_line(p);
	line = &p->lines[l];
	if (!save(p, &snap))
		return (false);
	for (int k = 0; k < line->count && s->found < MAX_SOLUTIONS; k++)
	{
		if (!snap.alive[l][k])
			continue ;
		restore(p, &snap);
		memset(line->alive, false, (size_t)line->count);	/* the guess: */
		line->alive[k] = true;								/* only k is left */
		line->alive_count = 1;
		s->guesses++;
		if (!search(p, s))
			return (drop(p, &snap), false);
	}
	restore(p, &snap);
	drop(p, &snap);
	return (true);
}

/*
 * Finds up to MAX_SOLUTIONS solutions. s->found tells how many:
 * 0 = no solution, 1 = unique, 2 = more than one.
 */
bool	solve(t_puzzle *p, t_search *s)
{
	s->found = 0;
	s->guesses = 0;
	return (search(p, s));
}
