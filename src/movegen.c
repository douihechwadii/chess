#include "../include/movegen.h"
#include <stdio.h>

void MoveList_Init(MoveList *list)
{
    list->count = 0;
}

void MoveList_Add(MoveList *list, int from, int to, int flags, int promotion)
{
    if (list->count > MAX_MOVES)
    {
        printf("Error: Move list overflow \n");
        return;
    }
    
    Move *move = &list->moves[list->count++];
    move->from = from;
    move->to = to;
    move->flags = flags;
    move->promotion = promotion;
    move->captured = PIECE_NONE;
}

int MoveGen_IsValidSquare(int rank, int file)
{
    return rank >= 0 && rank < BOARD_RANKS && file >= 0 && file < BOARD_FILES;
}

// raycasting (calculate all possible moves in one direction for sliding pieces)
void MoveGen_AddSlidingMoves(const Board *board, MoveList *list, int square, int color, int rankDir, int fileDir)
{
    int rank = Board_Rank(square);
    int file = Board_File(square);

    for (int i = 1; i < 8; i++)
    {
        int newRank = rank + i * rankDir;
        int newFile = file + i * fileDir;

        if (!MoveGen_IsValidSquare(newRank, newFile))
            break;

        int targetSquare = Board_Index(newRank, newFile);
        int targetPiece = Board_GetPiece(board, targetSquare);

        if (targetPiece == PIECE_NONE)
        {
            MoveList_Add(list, square, targetSquare, MOVE_NONE, PIECE_NONE);
        }
        else
        {
            if (Board_PieceColor(targetPiece) != color)
            {
                MoveList_Add(list, square, targetSquare, MOVE_CAPTURE, PIECE_NONE);
            }
            break;
        }
    }
}