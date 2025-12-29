#include <stdio.h>
#include "board.h"
#include "movegen.h"

int main(void)
{
    Board board;
    MoveList moveList;

    InitFromFEN(&board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
    Board_DebugPrint(&board);

    printf("\nChecking each square for white pieces:\n");
    for (int square = 0; square < BOARD_SIZE; square++)
    {
        int piece = Board_GetPiece(&board, square);
        if (piece != PIECE_NONE)
        {
            int color = Board_PieceColor(piece);
            int type = Board_PieceType(piece);
            
            char files[] = "abcdefgh";
            int rank = Board_Rank(square);
            int file = Board_File(square);
            
            const char* typeNames[] = {"None", "Pawn", "Rook", "Knight", "Bishop", "Queen", "King"};
            const char* colorNames[] = {"Black", "Neutral", "White"};
            
            printf("Square %c%d (index %d): %s %s (value=%d, color=%d)\n",
                   files[file], 8 - rank, square,
                   colorNames[color + 1], typeNames[type], piece, color);
            
            if (color == 1 && type == PAWN)
            {
                int direction = -1;
                int targetRank = rank + direction;
                int targetSquare = Board_Index(targetRank, file);
                int targetPiece = Board_GetPiece(&board, targetSquare);
                
                printf("  -> Target square in front: %c%d (index %d), piece value: %d\n",
                       files[file], 8 - targetRank, targetSquare, targetPiece);
                
                MoveList_Init(&moveList);
                MoveGen_GeneratePieceMoves(&board, &moveList, square, 1);
                printf("  -> Generated %d moves\n", moveList.count);
            }
            else if (color == 1)
            {
                MoveList_Init(&moveList);
                MoveGen_GeneratePieceMoves(&board, &moveList, square, 1);
                printf("  -> Generated %d moves\n", moveList.count);
            }
        }
    }

    printf("\n=== Generating all white moves ===\n");
    MoveGen_GenerateAllMoves(&board, &moveList, 1);
    printf("Total moves: %d\n\n", moveList.count);

    return 0;
}