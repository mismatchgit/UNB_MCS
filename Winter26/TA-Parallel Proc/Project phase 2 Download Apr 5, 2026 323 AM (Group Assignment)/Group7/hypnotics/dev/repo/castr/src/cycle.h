
#include <stdbool.h>

#ifndef CYCLIC_AUTOMATON
#define CYCLIC_AUTOMATON

/// The representation of a cyclic board
typedef struct {
    int *arr;
    int size;
    int threshold;
    int lines;
    int col;
    int states;
    // not yet implemented
    int range;
    bool isMoore;
} cyclic_automaton;

/// Mutates alt_board with the next sim step
void automaton_step(cyclic_automaton *board, cyclic_automaton *alt_board);

/// Duplicates a board
cyclic_automaton *dup_board(cyclic_automaton *board);

/// Creates a new board
cyclic_automaton * init_board(int *arr, int lines, int col, int threshold, int states, bool isMoore); 

/// Returns whether or not the point should be incremented, using the Von Newman
/// neighbourhood pattern
bool von_newman_neighbourhood(cyclic_automaton *board, int line, int col);

/// Returns whether or not the point should be incremented, using the Moore
/// neighbourhood pattern
bool moore_neighbourhood(cyclic_automaton *board, int line, int col);

/// Prints a board, for debugging purposes
void print_board(cyclic_automaton *board);

#endif /* CYCLIC_AUTOMATON */ 
