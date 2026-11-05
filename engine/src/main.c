#include <stdio.h>
#include <stdlib.h>
#include "engine.h"

/* A clue as printed: its digit, or '.' when blank. */
static char	clue_char(int clue)
{
	return (clue == 0 ? '.' : (char)('0' + clue));
}

/* Prints a solved grid with the clues around it, like your print_result. */
static void	print_grid(const t_puzzle *p, uint8_t grid[MAX_N][MAX_N])
{
	printf("\n     ");
	for (int c = 0; c < p->n; c++)
		printf(" %c", clue_char(p->top[c]));
	printf("\n");
	for (int r = 0; r < p->n; r++)
	{
		printf("  %c |", clue_char(p->left[r]));
		for (int c = 0; c < p->n; c++)
			printf("%d|", grid[r][c]);
		printf(" %c\n", clue_char(p->right[r]));
	}
	printf("     ");
	for (int c = 0; c < p->n; c++)
		printf(" %c", clue_char(p->bottom[c]));
	printf("\n\n");
}

/*
 * Read the clues, limit each cell using all four clues (step 1b), build each
 * line's candidates (step 1), let rows and columns filter each other
 * (step 2), and guess when that gets stuck (step 3).
 */
int	main(int argc, char **argv)
{
	t_puzzle	p;
	t_search	s;
	t_result	result;
	bool		ok;

	if (argc != 2 && argc != 3)
	{
		fprintf(stderr, "Usage: %s \"top... bottom... left... right...\" [\"grid\"]\n"
			"  clues: '.' or 0 for blank\n"
			"  grid (optional): the cells row by row, '.' for empty\n", argv[0]);
		return (EXIT_FAILURE);
	}
	if (!parse_clues(argv[1], &p))
		return (EXIT_FAILURE);
	apply_clue_limits(&p);
	if (argc == 3 && !apply_givens(argv[2], &p))
		return (EXIT_FAILURE);
	ok = build_lines(&p);
	if (ok)
	{
		printf("Grid size: %d x %d, %d candidates saved\n",
			p.n, p.n, total_alive(&p));
		result = propagate(&p, true);
		if (result == STUCK)
			printf("Stuck after propagation: guessing...\n");
		ok = solve(&p, &s);
	}
	if (!ok)
	{
		fprintf(stderr, "Error: out of memory.\n");
		free_lines(&p);
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
		print_grid(&p, s.solution[0]);
	}
	else
	{
		printf("This puzzle has more than one solution (%ld guesses). Two of them:\n",
			s.guesses);
		print_grid(&p, s.solution[0]);
		print_grid(&p, s.solution[1]);
	}
	free_lines(&p);
	return (s.found == 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
