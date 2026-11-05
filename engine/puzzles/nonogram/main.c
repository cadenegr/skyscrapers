#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "engine.h"

/*
 * The Nonogram plug-in.
 *
 * A Nonogram is a grid you colour in. The numbers for each row and column
 * give the blocks of filled squares in that line, in order, with at least
 * one empty square between two blocks. "3 1" on a 6-wide row could be
 * ###.#.  or  ###..#  or  .###.#  (and nothing else).
 *
 * For the engine, a cell is 0 (empty) or 1 (filled), and every row and
 * column is a group whose candidates are all the ways to place its blocks.
 * Propagation and guessing are the core's, unchanged from Skyscrapers.
 */

#define MAX_SIDE	25		/* widest grid we accept */
#define EMPTY		0
#define FILLED		1

typedef struct s_clue
{
	int	count;					/* how many blocks (0 = empty line) */
	int	block[MAX_SIDE];		/* their lengths, in order */
}	t_clue;

typedef struct s_nono
{
	int		rows;
	int		cols;
	t_clue	row[MAX_SIDE];		/* left to right */
	t_clue	col[MAX_SIDE];		/* top to bottom */
}	t_nono;

/*
 * Reads lines separated by '/', each a list of block lengths:
 * "1 1 / 3 / 1 1" is three lines. "0" is an empty line.
 */
static int	parse_lines(const char *s, t_clue *out, const char *what)
{
	int	n = 0;

	out[0].count = 0;
	while (1)
	{
		if (*s == '/' || *s == '\0')
		{
			if (++n > MAX_SIDE)
				return (fprintf(stderr, "Error: more than %d %s.\n", MAX_SIDE, what), -1);
			if (*s == '\0')
				return (n);
			out[n].count = 0;
			s++;
		}
		else if (isspace((unsigned char)*s) || *s == ',')
			s++;
		else if (isdigit((unsigned char)*s))
		{
			char	*end;
			long	len = strtol(s, &end, 10);

			s = end;
			if (len == 0)
				continue ;				/* "0" means an empty line */
			if (len > MAX_SIDE || out[n].count == MAX_SIDE)
				return (fprintf(stderr, "Error: block too long in %s.\n", what), -1);
			out[n].block[out[n].count++] = (int)len;
		}
		else
			return (fprintf(stderr, "Error: unexpected character '%c'.\n", *s), -1);
	}
}

/* Everything the generator needs to know about the line it is building. */
typedef struct s_gen
{
	t_engine		*e;
	int				group;
	int				len;
	const t_clue	*clue;
	t_set			cell[MAX_SIDE];		/* allowed values for each cell of the line */
	int				need[MAX_SIDE + 1];	/* cells needed by blocks i.. (with gaps) */
	uint8_t			vals[MAX_SIDE];
}	t_gen;

/*
 * Places block `b` at every position from `start` on, then the next blocks
 * after it. Like the Skyscrapers generator, each cell is checked as soon as
 * it is decided, so a placement that puts a filled square where the cell
 * must be empty (or the other way round) is dropped right away.
 */
static bool	place(t_gen *g, int b, int start)
{
	if (b == g->clue->count)
	{
		for (int i = start; i < g->len; i++)
		{
			if (!HAS(g->cell[i], EMPTY))
				return (true);
			g->vals[i] = EMPTY;
		}
		return (engine_add_candidate(g->e, g->group, g->vals));
	}
	for (int s = start; s + g->need[b] <= g->len; s++)
	{
		int		end = s + g->clue->block[b];
		bool	fits = true;

		if (s > start)
		{
			if (!HAS(g->cell[s - 1], EMPTY))
				break ;					/* that cell must be filled: can't slide past it */
			g->vals[s - 1] = EMPTY;
		}
		for (int i = s; i < end && fits; i++)
			fits = HAS(g->cell[i], FILLED);
		if (!fits)
			continue ;
		for (int i = s; i < end; i++)
			g->vals[i] = FILLED;
		if (b + 1 < g->clue->count)
		{
			if (!HAS(g->cell[end], EMPTY))
				continue ;				/* need a gap after the block */
			g->vals[end] = EMPTY;
			end++;
		}
		if (!place(g, b + 1, end))
			return (false);
	}
	return (true);
}

static bool	build_line(t_engine *e, const t_clue *clue, const int *cells, int len)
{
	t_gen	g;

	g.e = e;
	g.len = len;
	g.clue = clue;
	g.group = engine_add_group(e, cells, len);
	if (g.group < 0)
		return (false);
	g.need[clue->count] = 0;
	for (int b = clue->count - 1; b >= 0; b--)
		g.need[b] = clue->block[b] + (b + 1 < clue->count ? 1 + g.need[b + 1] : 0);
	for (int i = 0; i < len; i++)
		g.cell[i] = e->domain[cells[i]];
	return (place(&g, 0, 0) && engine_finish_group(e, g.group));
}

static bool	build_lines(const t_nono *p, t_engine *e)
{
	int	cells[MAX_SIDE];

	for (int r = 0; r < p->rows; r++)
	{
		for (int c = 0; c < p->cols; c++)
			cells[c] = r * p->cols + c;
		if (!build_line(e, &p->row[r], cells, p->cols))
			return (false);
	}
	for (int c = 0; c < p->cols; c++)
	{
		for (int r = 0; r < p->rows; r++)
			cells[r] = r * p->cols + c;
		if (!build_line(e, &p->col[c], cells, p->rows))
			return (false);
	}
	return (true);
}

/* Prints the picture: column clues stacked on top, row clues on the left. */
static void	print_picture(const t_nono *p, const uint8_t *grid)
{
	int	tallest = 1;
	int	widest = 1;
	int	w = 2;		/* width of one column: 3 if any column clue has two digits */

	for (int c = 0; c < p->cols; c++)
	{
		if (p->col[c].count > tallest)
			tallest = p->col[c].count;
		for (int b = 0; b < p->col[c].count; b++)
			if (p->col[c].block[b] >= 10)
				w = 3;
	}
	for (int r = 0; r < p->rows; r++)
		if (p->row[r].count * 3 > widest)
			widest = p->row[r].count * 3;
	printf("\n");
	for (int line = 0; line < tallest; line++)
	{
		printf("%*s  ", widest, "");
		for (int c = 0; c < p->cols; c++)
		{
			int	skip = tallest - p->col[c].count;

			if (line >= skip)
				printf("%*d", w, p->col[c].block[line - skip]);
			else
				printf("%*s", w, "");
		}
		printf("\n");
	}
	for (int r = 0; r < p->rows; r++)
	{
		char	clue[MAX_SIDE * 3 + 2] = "";
		int		at = 0;

		for (int b = 0; b < p->row[r].count; b++)
			at += snprintf(clue + at, sizeof(clue) - (size_t)at, " %d", p->row[r].block[b]);
		printf("%*s  ", widest, p->row[r].count ? clue : " 0");
		for (int c = 0; c < p->cols; c++)
			printf("%*c", w, grid[r * p->cols + c] == FILLED ? '#' : '.');
		printf("\n");
	}
	printf("\n");
}

int	main(int argc, char **argv)
{
	t_nono		p;
	t_engine	e;
	t_solutions	s = {0};
	bool		ok;

	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s \"row clues\" \"column clues\"\n"
			"  lines separated by '/', blocks by spaces, 0 = empty line\n"
			"  example: %s \"1 1 / 3 / 1 1\" \"3 / 1 / 3\"\n", argv[0], argv[0]);
		return (EXIT_FAILURE);
	}
	p.rows = parse_lines(argv[1], p.row, "rows");
	p.cols = parse_lines(argv[2], p.col, "columns");
	if (p.rows < 0 || p.cols < 0)
		return (EXIT_FAILURE);
	if (!engine_init(&e, p.rows * p.cols, BIT(EMPTY) | BIT(FILLED)))
		return (fprintf(stderr, "Error: out of memory.\n"), EXIT_FAILURE);
	ok = build_lines(&p, &e);
	if (ok)
	{
		printf("Grid: %d rows x %d columns, %d candidates saved\n",
			p.rows, p.cols, engine_total_alive(&e));
		if (engine_propagate(&e, true) == STUCK)
			printf("Stuck after propagation: guessing...\n");
		ok = engine_solve(&e, &s, 2);
	}
	if (!ok)
	{
		fprintf(stderr, "Error: out of memory.\n");
		return (solutions_free(&s), engine_free(&e), EXIT_FAILURE);
	}
	if (s.found == 0)
		printf("No solution: these clues contradict each other.\n");
	else if (s.found == 1)
	{
		if (s.guesses == 0)
			printf("Solved by propagation alone.\n");
		else
			printf("Solved after %ld guesses. The solution is unique.\n", s.guesses);
		print_picture(&p, s.grid);
	}
	else
	{
		printf("This puzzle has more than one solution (%ld guesses). Two of them:\n",
			s.guesses);
		print_picture(&p, &s.grid[0]);
		print_picture(&p, &s.grid[e.ncells]);
	}
	ok = s.found > 0;
	solutions_free(&s);
	engine_free(&e);
	return (ok ? EXIT_SUCCESS : EXIT_FAILURE);
}
