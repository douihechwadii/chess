#include <stdio.h>
#include "board.h"

int main(void)
{
    Board board;
    Move move;

    Board_InitFromFEN(&board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");

    printf("Initial Position: \n");
    Board_DebugPrint(&board);

    move.from = Board_Index(6, 4);
    move.to = Board_Index(4, 4);
    move.flags = MOVE_NONE;
    
    Board_DoMove(&board, &move);

    Board_DebugPrint(&board);

    Board_UndoMove(&board, &move);

    Board_DebugPrint(&board);

    move.to = Board_Index(0, 4);
    move.flags = MOVE_PROMOTION;
    move.promotion = QUEEN;
    
    Board_DoMove(&board, &move);
    Board_DebugPrint(&board);

    Board_UndoMove(&board, &move);

    Board_DebugPrint(&board);

}