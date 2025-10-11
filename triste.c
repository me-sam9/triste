/* See LICENSE for license details. */

/* triste, a tetris clone written by sam9 */

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define SPEED 200000

#define GAME_TPADDING 4
#define GAME_LPADDING 8

#define ROWS 24
#define COLS 24
#define GAMEROWS ROWS
#define GAMECOLS (COLS / 2)

#define PIECESIZE 4

int pieces[][PIECESIZE][PIECESIZE] = {
	{
		{ 2, 0, 0, 2 },
		{ 0, 0, 0, 0 },
		{ 1, 1, 1, 1 },
		{ 0, 0, 0, 0 },
	},
	{
		{ 2, 0, 0, 2 },
		{ 0, 1, 0, 0 },
		{ 1, 1, 1, 0 },
		{ 0, 0, 0, 0 },
	},
	{
		{ 2, 0, 0, 2 },
		{ 0, 0, 1, 0 },
		{ 1, 1, 1, 0 },
		{ 0, 0, 0, 0 },
	},
	{
		{ 2, 0, 0, 2 },
		{ 1, 0, 0, 0 },
		{ 1, 1, 1, 0 },
		{ 0, 0, 0, 0 },
	},
	{
		{ 2, 0, 0, 2 },
		{ 0, 1, 1, 0 },
		{ 0, 1, 1, 0 },
		{ 0, 0, 0, 0 },
	},
	{
		{ 2, 0, 0, 2 },
		{ 1, 1, 0, 0 },
		{ 0, 1, 1, 0 },
		{ 0, 0, 0, 0 },
	},
	{
		{ 2, 0, 0, 2 },
		{ 0, 1, 1, 0 },
		{ 1, 1, 0, 0 },
		{ 0, 0, 0, 0 },
	},
};

enum {
	EMPTY = 0,
	PIXEL = 1,
	PIVOT = 2,
	GROUND = 4,
};

int game[GAMEROWS][GAMECOLS];
int points = 0;

/* deletes all the full lines and increments the points counter accordingly. */
void delete_lines(void);

/* 
 * detects if a piece touched the ground. If it did, transforms it into ground
 * and calls delete_lines() to clear the full lines.
 */
int impact(void);

/* moves the pieces 1 pixel down. It assumes impact() was called before. */
void gravitate(void);

/* moves the pieces 1 pixel to the left. */
void mvleft(void);

/* moves the pieces 1 pixel to the right. */
void mvright(void);

/* 
 * transposes the 4x4 submatrix of the game matrix starting at the game[i][j]
 * position.
 */
void tranpose(int i, int j);

/* 
 * reverses the rows of the 4x4 submatrix of the game matrix starting at the
 * game[i][j] position.
 */
void reverse_rows(int i, int j);

/* 
 * rotates the 4x4 submatrix of the game matrix starting at the first position
 * with a PIVOT, then it rearranges the PIVOTs to preserve their original
 * position.
 */
void rotate(void);

/* generates a random piece and places it in the top middle of the game */
int nextpiece(void);

/* prints the current state of the game matrix to the gamew window. */
void wprint_game(WINDOW *gamew);

/* 
 * finishes the game, prints a "game over" message and shows the accumulated
 * points.
 */
void endgame();

/* 
 * handles user input to perform the following actions:
 * move left, move right, rotate clockwise, and accelerate descent.
 */
void input_handler(int c);

int
main(void)
{
	initscr();
	noecho();
	keypad(stdscr, TRUE);
	curs_set(0);

	cbreak();
	nodelay(stdscr, TRUE);

	srand(time(NULL));

	WINDOW *gamew = newwin(ROWS + 2, COLS + 2, GAME_TPADDING, GAME_LPADDING);
	refresh();
	box(gamew, 0, 0);

	nextpiece();
	while (1) {
		wprint_game(gamew);
		wrefresh(gamew);

		usleep(SPEED);

		int c = getch();
		if (c == 'q')
			break;
		input_handler(c);

		if (impact() && !nextpiece()) {
			endgame();
			break;
		}
		gravitate();
	}

	endwin();
	return 0;
}

void
delete_lines(void)
{
	for (int i = 0; i < GAMEROWS; i++) {
		for (int j = 0; j < GAMECOLS; j++) {
			int *px = &game[GAMEROWS - 1 - i][j];

			if (*px != GROUND && i < GAMEROWS - 1) {
				j = 0;
				i++;
			}
			else if (*px & GROUND && j == GAMECOLS - 1) {
				for (int k = i + 1; k < GAMEROWS; k++) {
					for (int l = 0; l < GAMECOLS; l++) {
						int *px = &game[GAMEROWS - 1 - k][l];
						int *dw = &game[GAMEROWS - k][l];

						*dw = *px;
						*px = EMPTY;
					}
				}
				i--;
				points++;
			}
		}
	}
}

int
impact(void)
{
	for (int i = 0; i < GAMEROWS; i++) {
		for (int j = 0; j < GAMECOLS; j++) {
			int *up = &game[GAMEROWS - 1 - i][j];
			int *dw = &game[GAMEROWS - i][j];

			if (*up == PIXEL && (*dw & GROUND || i == 0)) {
				for (int k = 0; k < GAMEROWS; k++) {
					for (int l = 0; l < GAMECOLS; l++) {
						int px = game[k][l];
						if (px & PIVOT) {
							game[k][l] &= ~PIVOT;
						}
						else if (px & PIXEL) {
							game[k][l] = GROUND;
						}
					}
				}
				delete_lines();
				return 1;
			}
		}
	}
	return 0;
}

void
gravitate(void)
{
	for (int i = 0; i < GAMEROWS; i++) {
		for (int j = 0; j < GAMECOLS; j++) {
			int *px = &game[GAMEROWS - 1 - i][j];
			int *dw = &game[GAMEROWS - i][j];

			if (*px & PIXEL) {
				*px &= ~PIXEL;
				*dw |= PIXEL;
			}
			else if (*px & PIVOT) {
				*px &= ~PIVOT;
				*dw |= PIVOT;
			}
		}
	}
}

void
mvleft(void)
{
	for (int i = 0; i < GAMEROWS; i++) {
		if (game[i][0] & PIXEL) {
			return;
		}
		for (int j = 1; j < GAMECOLS; j++) {
			int px = game[i][j];
			int left = game[i][j - 1];
			if (px & PIXEL && left & GROUND)
				return;
      		}
	}
	for (int i = 0; i < GAMEROWS; i++) {
		int pivots = 0;
		int location = 0;
		for (int j = 0; j < GAMECOLS; j++) {
			int px = game[i][j];
			if (px & PIXEL) {
				game[i][j - 1] = px;
				game[i][j] &= ~PIXEL;
			}
			else if (px & PIVOT) {
				if (j != 0)
					game[i][j - 1] |= PIVOT;
				game[i][j] &= ~PIVOT;

				pivots++;
				location = j - 1;
			}
		}
		if (pivots == 1 && location > GAMECOLS / 2 && location + PIECESIZE - 1 < GAMECOLS) {
			game[i][location + PIECESIZE - 1] |= PIVOT;
		}
	}
}

void
mvright(void)
{
	for (int i = 0; i < GAMEROWS; i++) {
		if (game[i][GAMECOLS - 1] & PIXEL) {
			return;
		}
		for (int j = 0; j < GAMECOLS - 1; j++) {
			int px = game[i][j];
			int right = game[i][j + 1];
			if (px & PIXEL && right & GROUND)
				return;
      		}
	}
	for (int i = 0; i < GAMEROWS; i++) {
		int pivots = 0;
		int location = 0;
		for (int j = 0; j < GAMECOLS; j++) {
			int px = game[i][GAMECOLS - 1 - j];
			if (px & PIXEL) {
				game[i][GAMECOLS - j] = px;
				game[i][GAMECOLS - 1 - j] &= ~PIXEL;
			}
			else if (px & PIVOT) {
				if (j != 0)
					game[i][GAMECOLS - j] |= PIVOT;
				game[i][GAMECOLS - 1 - j] &= ~PIVOT;

				pivots++;
				location = GAMECOLS - j;
			}
		}
		if (pivots == 1 && location < GAMECOLS / 2 && location - (PIECESIZE - 1) >= 0) {
			game[i][location - (PIECESIZE - 1)] |= PIVOT;
		}
	}
}

void
tranpose(int i, int j)
{
	for (int k = 0; k < PIECESIZE; k++) {
		for (int l = k + 1; l < PIECESIZE; l++) {
			int tmp = game[i + k][j + l] & PIXEL;

			game[i + k][j + l] &= ~PIXEL;
			game[i + k][j + l] |= game[i + l][j + k] & PIXEL;

			game[i + l][j + k] &= ~PIXEL;
			game[i + l][j + k] |= tmp;

		}
	}
}

void
reverse_rows(int i, int j)
{
	for (int k = 0; k < PIECESIZE; k++) {
		for (int l = 0; l < PIECESIZE / 2; l++) {
			int tmp = game[i + k][j + l] & PIXEL;

			game[i + k][j + l] &= ~PIXEL;
			game[i + k][j + l] |= game[i + k][j + PIECESIZE - 1 - l] & PIXEL;

			game[i + k][j + PIECESIZE - 1 - l] &= ~PIXEL;
			game[i + k][j + PIECESIZE - 1 - l] |= tmp;
		}
	}
}

void
rotate(void)
{
	for (int i = 0; i < GAMEROWS; i++) {
		for (int j = 0; j < GAMECOLS; j++) {

			if (!(game[i][j] & PIVOT))
				continue;

			int second_pivot = 0;
			for (int k = j + PIECESIZE - 1; k < GAMECOLS; k++) {
				if (game[i][k] & PIVOT) {
					second_pivot = 1;
					break;
				}
			}

			if (!second_pivot) {
				if (j < GAMECOLS / 2) {
					mvright();
				}
				else {
					mvleft();
				}
				rotate();
				return;
			}

			reverse_rows(i, j);
			tranpose(i, j);
			game[i][j + PIECESIZE - 1] |= PIVOT;
			game[i + PIECESIZE - 1][j] &= ~PIVOT;

			for (int k = 0; k < PIECESIZE; k++) {
				for (int l = 0; l < PIECESIZE; l++) {
					int px = game[i + k][j + l];
					if (px & PIXEL && px & GROUND) {
						tranpose(i, j);
						reverse_rows(i, j);
					}
				}
			}
			return;
		}
	}
}

int
nextpiece(void)
{
	int piece = rand() % 7;
	for (int i = 0; i < PIECESIZE; i++) {
		for (int j = 0; j < PIECESIZE; j++) {
			if (game[i][j + GAMECOLS / 2 - PIECESIZE / 2] & GROUND) {
				return 0;
			}
			game[i][j + GAMECOLS / 2 - PIECESIZE / 2] = pieces[piece][i][j];
		}
	}
	return 1;
}

void
wprint_game(WINDOW *gamew)
{
	wmove(gamew, 1, 1);
	for (int i = 0; i < GAMEROWS; i++) {
		for (int j = 0; j < GAMECOLS; j++) {
			int px = game[i][j];
			if (px == EMPTY || px == PIVOT)
				wprintw(gamew, "  ");
			else
				wprintw(gamew, "[]");
		}
		wmove(gamew, i + 2, 1);
	}
}

void
endgame(void)
{
	nodelay(stdscr, FALSE);

	WINDOW *popupw = newwin(4, 13, ROWS / 2 + 3, COLS / 2 + 3);
	refresh();
	box(popupw, 0, 0);
	mvwprintw(popupw, 1, 1, " GAME OVER ");
	mvwprintw(popupw, 2, 1, "%04d POINTS", points);
	wrefresh(popupw);

	while (getch() != 'q');
}

void
input_handler(int c)
{
	switch (c) {
		case KEY_LEFT:
		case 'h':
		case 'H':
			mvleft();
			break;
		case KEY_DOWN:
		case 'j':
		case 'J':
			if (impact() && !nextpiece())
				endgame();
			gravitate();
			break;
		case KEY_UP:
		case 'k':
		case 'K':
			rotate();
			break;
		case KEY_RIGHT:
		case 'l':
		case 'L':
			mvright();
			break;
	}
}
