#include "../include/movegen.h"
#include <stdio.h>

void MoveList_Init(MoveList *list)
{
    list->count = 0;
}

void MoveList_Add(MoveList *list, int from, int to, int flags, int promotion)
{
    if (list->count >= MAX_MOVES)
    {
        printf("Warning: Move list overflow\n");
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
    return (rank >= 0 && rank < BOARD_RANKS && file >= 0 && file < BOARD_FILES);
}

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

void MoveGen_GeneratePawnMoves(const Board *board, MoveList *list, int square, int color, int enPassantSquare)
{
    int rank = Board_Rank(square);
    int file = Board_File(square);
    int direction = (color > 0) ? -1 : 1;
    int startRank = (color > 0) ? 6 : 1;
    int promotionRank = (color > 0) ? 0 : 7;

    // Forward move
    int newRank = rank + direction;
    if (MoveGen_IsValidSquare(newRank, file))
    {
        int targetSquare = Board_Index(newRank, file);
        int targetPiece = Board_GetPiece(board, targetSquare);

        if (targetPiece == PIECE_NONE)
        {
            if (newRank == promotionRank)
            {
                // Promotions
                MoveList_Add(list, square, targetSquare, MOVE_PROMOTION, QUEEN);
                MoveList_Add(list, square, targetSquare, MOVE_PROMOTION, ROOK);
                MoveList_Add(list, square, targetSquare, MOVE_PROMOTION, BISHOP);
                MoveList_Add(list, square, targetSquare, MOVE_PROMOTION, KNIGHT);
            }
            else
            {
                MoveList_Add(list, square, targetSquare, MOVE_NONE, PIECE_NONE);

                // Double push from starting position
                if (rank == startRank)
                {
                    int doubleRank = rank + 2 * direction;
                    int doubleSquare = Board_Index(doubleRank, file);
                    if (Board_GetPiece(board, doubleSquare) == PIECE_NONE)
                    {
                        MoveList_Add(list, square, doubleSquare, MOVE_NONE, PIECE_NONE);
                    }
                }
            }
        }
    }

    // Captures
    int captureDirs[2] = {-1, 1};
    for (int i = 0; i < 2; i++)
    {
        int captureFile = file + captureDirs[i];
        if (MoveGen_IsValidSquare(newRank, captureFile))
        {
            int targetSquare = Board_Index(newRank, captureFile);
            int targetPiece = Board_GetPiece(board, targetSquare);

            if (targetPiece != PIECE_NONE && Board_PieceColor(targetPiece) != color)
            {
                if (newRank == promotionRank)
                {
                    // Capture promotions
                    MoveList_Add(list, square, targetSquare, MOVE_CAPTURE | MOVE_PROMOTION, QUEEN);
                    MoveList_Add(list, square, targetSquare, MOVE_CAPTURE | MOVE_PROMOTION, ROOK);
                    MoveList_Add(list, square, targetSquare, MOVE_CAPTURE | MOVE_PROMOTION, BISHOP);
                    MoveList_Add(list, square, targetSquare, MOVE_CAPTURE | MOVE_PROMOTION, KNIGHT);
                }
                else
                {
                    MoveList_Add(list, square, targetSquare, MOVE_CAPTURE, PIECE_NONE);
                }
            }

            // En passant
            if (enPassantSquare >= 0 && targetSquare == enPassantSquare)
            {
                MoveList_Add(list, square, targetSquare, MOVE_EN_PASSANT | MOVE_CAPTURE, PIECE_NONE);
            }
        }
    }
}

void MoveGen_GenerateKnightMoves(const Board *board, MoveList *list, int square, int color)
{
    int rank = Board_Rank(square);
    int file = Board_File(square);

    int knightMoves[8][2] = {
        {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2}, {1, -2}, {1, 2}, {2, -1}, {2, 1}};

    for (int i = 0; i < 8; i++)
    {
        int newRank = rank + knightMoves[i][0];
        int newFile = file + knightMoves[i][1];

        if (MoveGen_IsValidSquare(newRank, newFile))
        {
            int targetSquare = Board_Index(newRank, newFile);
            int targetPiece = Board_GetPiece(board, targetSquare);

            if (targetPiece == PIECE_NONE)
            {
                MoveList_Add(list, square, targetSquare, MOVE_NONE, PIECE_NONE);
            }
            else if (Board_PieceColor(targetPiece) != color)
            {
                MoveList_Add(list, square, targetSquare, MOVE_CAPTURE, PIECE_NONE);
            }
        }
    }
}

void MoveGen_GenerateBishopMoves(const Board *board, MoveList *list, int square, int color)
{
    MoveGen_AddSlidingMoves(board, list, square, color, -1, -1);
    MoveGen_AddSlidingMoves(board, list, square, color, -1, 1);
    MoveGen_AddSlidingMoves(board, list, square, color, 1, -1);
    MoveGen_AddSlidingMoves(board, list, square, color, 1, 1);
}

void MoveGen_GenerateRookMoves(const Board *board, MoveList *list, int square, int color)
{
    MoveGen_AddSlidingMoves(board, list, square, color, -1, 0);
    MoveGen_AddSlidingMoves(board, list, square, color, 1, 0);
    MoveGen_AddSlidingMoves(board, list, square, color, 0, -1);
    MoveGen_AddSlidingMoves(board, list, square, color, 0, 1);
}

void MoveGen_GenerateQueenMoves(const Board *board, MoveList *list, int square, int color)
{
    MoveGen_GenerateBishopMoves(board, list, square, color);
    MoveGen_GenerateRookMoves(board, list, square, color);
}

void MoveGen_GenerateKingMoves(const Board *board, MoveList *list, int square, int color, int castlingRights)
{
    int rank = Board_Rank(square);
    int file = Board_File(square);

    int kingMoves[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};

    for (int i = 0; i < 8; i++)
    {
        int newRank = rank + kingMoves[i][0];
        int newFile = file + kingMoves[i][1];

        if (MoveGen_IsValidSquare(newRank, newFile))
        {
            int targetSquare = Board_Index(newRank, newFile);
            int targetPiece = Board_GetPiece(board, targetSquare);

            if (targetPiece == PIECE_NONE)
            {
                MoveList_Add(list, square, targetSquare, MOVE_NONE, PIECE_NONE);
            }
            else if (Board_PieceColor(targetPiece) != color)
            {
                MoveList_Add(list, square, targetSquare, MOVE_CAPTURE, PIECE_NONE);
            }
        }
    }

    // Castling 
    if (castlingRights > 0)
    {
        int baseRank = (color > 0) ? 7 : 0;

        // King side castling
        if ((castlingRights & (color > 0 ? 1 : 4)) && rank == baseRank && file == 4)
        {
            if (Board_GetPiece(board, Board_Index(baseRank, 5)) == PIECE_NONE &&
                Board_GetPiece(board, Board_Index(baseRank, 6)) == PIECE_NONE)
            {
                int targetSquare = Board_Index(baseRank, 6);
                MoveList_Add(list, square, targetSquare, MOVE_CASTLING, PIECE_NONE);
            }
        }

        // Queen side castling
        if ((castlingRights & (color > 0 ? 2 : 8)) && rank == baseRank && file == 4)
        {
            if (Board_GetPiece(board, Board_Index(baseRank, 3)) == PIECE_NONE &&
                Board_GetPiece(board, Board_Index(baseRank, 2)) == PIECE_NONE &&
                Board_GetPiece(board, Board_Index(baseRank, 1)) == PIECE_NONE)
            {
                int targetSquare = Board_Index(baseRank, 2);
                MoveList_Add(list, square, targetSquare, MOVE_CASTLING, PIECE_NONE);
            }
        }
    }
}

// MODIFIED: Now accepts en passant and castling parameters
void MoveGen_GeneratePieceMoves(const Board *board, MoveList *list, int square, int color, int enPassantSquare, int castlingRights)
{
    int piece = Board_GetPiece(board, square);
    if (piece == PIECE_NONE || Board_PieceColor(piece) != color)
        return;

    int pieceType = Board_PieceType(piece);

    switch (pieceType)
    {
    case PAWN:
        MoveGen_GeneratePawnMoves(board, list, square, color, enPassantSquare);
        break;
    case KNIGHT:
        MoveGen_GenerateKnightMoves(board, list, square, color);
        break;
    case BISHOP:
        MoveGen_GenerateBishopMoves(board, list, square, color);
        break;
    case ROOK:
        MoveGen_GenerateRookMoves(board, list, square, color);
        break;
    case QUEEN:
        MoveGen_GenerateQueenMoves(board, list, square, color);
        break;
    case KING:
        MoveGen_GenerateKingMoves(board, list, square, color, castlingRights);
        break;
    }
}

// MODIFIED: Now accepts en passant and castling parameters
void MoveGen_GenerateAllMoves(const Board *board, MoveList *list, int color, int enPassantSquare, int castlingRights)
{
    MoveList_Init(list);

    for (int square = 0; square < BOARD_SIZE; square++)
    {
        int piece = Board_GetPiece(board, square);
        if (piece != PIECE_NONE && Board_PieceColor(piece) == color)
        {
            MoveGen_GeneratePieceMoves(board, list, square, color, enPassantSquare, castlingRights);
        }
    }
}