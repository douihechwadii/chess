#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "move.h"
#include "board.h"

#define MAX_MOVES 256


typedef struct 
{
    Move moves[MAX_MOVES];
    int count;
} MoveList;


void MoveList_Init(MoveList *list);

void MoveList_Add(MoveList *list, int from, int to, int flags, int promotion);

int MoveGen_IsValidSquare(int rank, int file);

void MoveGen_AddSlidingMoves(const Board *board, MoveList *list, int square, int color, int rankDir, int fileDir);

#endif