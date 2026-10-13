#include <stdio.h>
#include <stdlib.h>
#include "../cycle.h"
#include "../image-to-video.h"

cyclic_automaton *read_in(char *file_name) {
    FILE *fp = fopen(file_name, "r");

    if (fp == NULL){
        fprintf(stderr, "Something went wrong opening this file\n");
        exit(EXIT_FAILURE);
    }

    int lines;
    int states;
    fread(&states, sizeof(int), 1, fp);
    fread(&lines, sizeof(int), 1, fp);

    int *data = malloc(sizeof(int) * lines * lines);
    fread(data, sizeof(int), lines * lines, fp);

    fclose(fp);

    cyclic_automaton *board = init_board(data, lines, lines, 0, states, true);
    board->range = 1;

    free(data);

    return board;
}

int main(int argc, char **argv) {
    printf("Welcome to CASTR\n");
    if (argc != 5){
        fprintf(stderr, "Error: incorrect usage <in-file> <out-file> <steps> <threshold>");
        exit(EXIT_FAILURE);
    }

    cyclic_automaton *board = read_in(argv[1]);
    int steps = atoi(argv[3]);

    // Set neighbourhood pattern
    board->isMoore = true;

    // Set threshold
    board->threshold = atoi(argv[4]);
    printf("Board dimensions: (%dx%d)\n", board->col, board->lines);

    cyclic_automaton* alt_board = dup_board(board);
    alt_board->range = 2;
    cyclic_automaton* swap;

    for (int i = 0; i < steps; ++i) {
        write_frame(i, board->arr, board->lines, -1);
        automaton_step(board,alt_board);

        // Swapping buffers
        swap = alt_board;
        alt_board = board;
        board = swap;
    }

    convert_to(argv[2], -1);

}

