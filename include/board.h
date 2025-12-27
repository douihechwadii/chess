#ifndef BOARD_H
#define BOARD_H

#define BOARD_SIZE 64
#define BOARD_RANKS 8
#define BOARD_FILES 8

#include "move.h"

typedef enum {
    PIECE_NONE = 0,

    PAWN = 1,
    ROOK = 2,
    KNIGHT = 3,
    BISHOP = 4,
    QUEEN = 5,
    KING = 6
} PieceType;

typedef struct 
{
    int squares[BOARD_SIZE];
} Board;

void Board_Clear(Board *board);
void Board_InitFromFEN(Board *board, const char *fen);

int Board_GetPiece(const Board *board, int index);
void Board_SetPiece(Board *board, int index, int value);

int Board_Index(int rank, int file);
int Board_Rank(int index);
int Board_File(int index);

int Board_PieceType(int value);
int Board_PieceColor(int value);

void Board_DebugPrint(const Board *board);

void Board_DoMove(Board *board, Move *move);
void Board_UndoMove(Board *board, const Move *move);

#endif