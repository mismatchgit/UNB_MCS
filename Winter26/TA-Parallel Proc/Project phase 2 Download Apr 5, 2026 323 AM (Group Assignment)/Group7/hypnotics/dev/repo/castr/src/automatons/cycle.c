
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "../cycle.h"

/// Return a properly index using modular arithmetic
static int inline idx(int i, int len) {
    return ((i) % (len) + (len)) % (len);
}

/// Return a point index using Row-Major-Indexing
static int inline point(int line, int col, int len) {
    return (line * len) + col;
}

static int inline wrap(int line, int col, cyclic_automaton *b) {
    return point(idx(line, b->lines), idx(col, b->col), b->col);
}


// TODO: implement range
bool moore_neighbourhood(cyclic_automaton *board, int line, int col) {

    const int moore_offsets[8][2] = {
        {-1, -1},{-1,0},{-1,1},
        {0, -1},        {0,1},
        {1, -1},{1,0},{1,1}
    };

    int n_count = 0;
    int basis = (board->arr[point(line, col, board->col)] + 1) % board->states;

    for (int i = 0; i < 8; ++i) {
        int l = line + moore_offsets[i][0];
        int c = col  + moore_offsets[i][1];

        if (board->arr[wrap(l,c,board)] == basis)
            n_count++;
    }

    return n_count >= board->threshold;
}

// TODO: implement range
bool von_newman_neighbourhood(cyclic_automaton *board, int line, int col) {

    int n_count = 0;
    int basis = (board->arr[point(line, col, board->col)] + 1) % board->states;

    // Formater will not allow me to make a visual pattern for these offsets,
    // but there supposed to be a cross.
    const int von_newman_offsets[4][2] = {
        {-1, 0}, {0, -1}, {0, 1}, {1, 0}
    };

    for (int i = 0; i < 4; ++i) {
      int l = line + von_newman_offsets[i][0];
      int c = col  + von_newman_offsets[i][1];

      if (board->arr[wrap(l,c,board)] == basis)
        n_count++;
    }

    return n_count >= board->threshold;
}

cyclic_automaton *
init_board(int *arr, int lines, int col, int threshold, int states, bool isMoore) {

    cyclic_automaton *board = malloc(sizeof(cyclic_automaton));
    if (board == NULL) {
        fprintf(stderr, "Failed to allocate memory for board\n");
        exit(EXIT_FAILURE);
    }

    board->size = lines * col;
    board->threshold = threshold;
    board->states = states;
    board->isMoore = isMoore;
    board->range=1;
    board->lines = lines;
    board->col = col;

    board->arr = (int *)malloc(sizeof(int) * board->size);
    if (board->arr == NULL) {
        fprintf(stderr, "Failed to allocate memory for board\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < board->size; ++i) {
        board->arr[i] = arr[i];
    }

    return board;
}

cyclic_automaton *dup_board(cyclic_automaton *board) {

    cyclic_automaton *new_board = (cyclic_automaton*) malloc(sizeof(cyclic_automaton));

    new_board->size = board->size;
    new_board->threshold = board->threshold;
    new_board->range = board->range;
    new_board->states = board->states;
    new_board->isMoore = board->isMoore;
    new_board->lines = board->lines;
    new_board->col = board->col;

    new_board->arr = (int *)malloc(sizeof(int) * board->size);

    for (int i = 0; i < board->size; ++i)
        new_board->arr[i] = board->arr[i];

    return new_board;
}

void automaton_step(cyclic_automaton *board, cyclic_automaton *alt_board) {
    bool increment;
    int p;
    for (int i = 0; i < board->lines; ++i) {
        for (int j = 0; j < board->col; ++j) {
            if (board->isMoore)
                increment = moore_neighbourhood(board, i, j);
            else increment = von_newman_neighbourhood(board, i, j);

            p = point(i,j,board->col);
            if (increment) {
                alt_board->arr[p] = (board->arr[p] + 1) % board->states;
            } else
                alt_board->arr[p] = board->arr[p];
        }
    }
}

void print_board(cyclic_automaton *board) {
    for (int i = 0; i < board->lines; ++i) {
        for (int j = 0; j < board->col; ++j) {
            printf("%d ", board->arr[idx(point(i,j,board->col), board->size)]);
        }
        printf("\n");
    }
}
