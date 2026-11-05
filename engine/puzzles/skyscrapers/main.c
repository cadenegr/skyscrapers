#include <stdio.h>
#include <stdlib.h>
#include "skyscrapers.h"

/* A clue as printed: its digit, or '.' when blank. */
static char	clue_char(int clue)
{
	return (clue == 0 ? '.' : (char)('0' + clue));
}

/* Prints a solved grid with the clues around it, like your print_result. */
static void	print_grid(const t_sky *p, const uint8_t *grid)
{
	printf("\n     ");
	for (int c = 0; c < p->n; c++)
		printf(" %c", clue_char(p->top[c]));
	printf("\n");
	for (int r = 0; r < p->n; r++)
	{
		printf("  %c |", clue_char(p->left[r]));
		for (int c = 0; c < p->n; c++)
			printf("%d|", grid[r * p->n + c]);
		printf(" %c\n", clue_char(p->right[r]));
	}
	printf("     ");
	for (int c = 0; c < p->n; c++)
		printf(" %c", clue_char(p->bottom[c]));
	printf("\n\n");
}

static int	usage(const char *name)
{
	fprintf(stderr, "Usage: %s \"top... bottom... left... right...\" [\"grid\"]\n"
		"  clues: '.' or 0 for blank\n"
		"  grid (optional): the cells row by row, '.' for empty\n", name);
	return (EXIT_FAILURE);
}

/*
 * The Skyscrapers plug-in describes the puzzle to the engine:
 *   - one cell per square, starting with every height 1..n possible;
 *   - the clue limits (step 1b) and any given numbers;
 *   - one group per row and per column, with its candidates (step 1).
 * The engine does the rest: propagation (step 2) and guessing (step 3).
 */
int	main(int argc, char **argv)
{
	t_sky		sky;
	t_engine	e;
	t_solutions	s = {0};
	bool		ok;

	if (argc != 2 && argc != 3)
		return (usage(argv[0]));
	if (!parse_clues(argv[1], &sky))
		return (EXIT_FAILURE);
	if (!engine_init(&e, sky.n * sky.n, BIT(sky.n + 1) - 2))	/* bits 1..n */
		return (fprintf(stderr, "Error: out of memory.\n"), EXIT_FAILURE);
	apply_clue_limits(&sky, &e);
	if (argc == 3 && !apply_givens(argv[2], &sky, &e))
		return (engine_free(&e), EXIT_FAILURE);
	ok = build_lines(&sky, &e);
	if (ok)
	{
		printf("Grid size: %d x %d, %d candidates saved\n",
			sky.n, sky.n, engine_total_alive(&e));
		if (engine_propagate(&e, true) == STUCK)
			printf("Stuck after propagation: guessing...\n");
		ok = engine_solve(&e, &s, 2);
	}
	if (!ok)
	{
		fprintf(stderr, "Error: out of memory.\n");
		solutions_free(&s);
		engine_free(&e);
		return (EXIT_FAILURE);
	}
	if (s.found == 0)
		printf("No solution: these clues contradict each other.\n");
	else if (s.found == 1)
	{
		if (s.guesses == 0)
			printf("Solved by propagation alone.\n");
		else
			printf("Solved after %ld guesses. The solution is unique.\n", s.guesses);
		print_grid(&sky, s.grid);
	}
	else
	{
		printf("This puzzle has more than one solution (%ld guesses). Two of them:\n",
			s.guesses);
		print_grid(&sky, &s.grid[0]);
		print_grid(&sky, &s.grid[e.ncells]);
	}
	ok = s.found > 0;
	solutions_free(&s);
	engine_free(&e);
	return (ok ? EXIT_SUCCESS : EXIT_FAILURE);
}
