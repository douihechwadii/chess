#include "../include/board.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

static int FenCharToPieceValue(char c)
{
    switch (c)
    {
        case 'p': return -PAWN;
        case 'r': return -ROOK;
        case 'n': return -KNIGHT;
        case 'b': return -BISHOP;
        case 'q': return -QUEEN;
        case 'k': return -KING;
        
        case 'P': return PAWN;
        case 'R': return ROOK;
        case 'N': return KNIGHT;
        case 'B': return BISHOP;
        case 'Q': return QUEEN;
        case 'K': return KING;
        
        default: return PIECE_NONE;
    }
}

void Board_Clear(Board *board)
{
    memset(board->squares, 0, sizeof(board->squares));
}

void Board_InitFromFEN(Board *board, const char *fen)
{
    Board_Clear(board);
    int index = 0;

    for (int i = 0; fen[i] != '\0' && fen[i] != ' '; i++)
    {
        char c = fen[i];

        if (c == '/')
        {
            continue;
        }
        else if (isdigit((unsigned char)c))
        {
            index += c - '0';
        }
        else
        {
            board->squares[index++] = FenCharToPieceValue(c);
        }
        
    }
    
}

int Board_GetPiece(const Board *board, int index)
{
    return board->squares[index];
}

void Board_SetPiece(Board *board, int index, int value)
{
    board->squares[index] = value;
}

void Board_MovePiece(Board *board, int from, int to)
{
    board->squares[to] = board->squares[from];
    board->squares[from] = PIECE_NONE;
}

int Board_Index(int rank, int file)
{
    return rank * BOARD_FILES + file;
}

int Board_Rank(int index)
{
    return index / BOARD_FILES;
}

int Board_File(int index)
{
    return index % BOARD_FILES;
}

int Board_PieceType(int value)
{
    return value < 0 ? -value: value;
}

int Board_PieceColor(int value)
{
    if(value > 0) return 1;
    if(value < 0) return -1;
    return 0;
}

void Board_DebugPrint(const Board *board)
{
    printf("\n");

    for (int rank = 0; rank < BOARD_RANKS; rank++)
    {
        printf("%d ", 8 - rank);

        for (int file = 0; file < BOARD_FILES; file++)
        {
            int index = Board_Index(rank, file);
            int value = board->squares[index];

            char c = '.';

            switch (Board_PieceType(value))
            {
                case PAWN: c = 'p'; break;
                case ROOK: c = 'r'; break;
                case KNIGHT: c = 'n'; break;
                case BISHOP: c = 'b'; break;
                case QUEEN: c = 'q'; break;
                case KING: c = 'k'; break;
                default: c = '.'; break;
            }
            if (value > 0)
            {
                c = (char)toupper(c);
            }
            printf("%c ", c);
        }
        printf("\n");
    }
    printf("\n  a b c d e f g h\n\n");
}

void Board_DoMove(Board *board, Move *move)
{
    int movingPiece = board->squares[move->from];
    int targetPiece = board->squares[move->to];

    /* Save captured piece for undo */
    move->captured = targetPiece;

    /* =======================
       Special cases
       ======================= */

    /* En passant capture */
    if (move->flags & MOVE_EN_PASSANT)
    {
        int dir = (movingPiece > 0) ? 1 : -1;
        int capturedIndex = move->to + dir * BOARD_FILES;

        move->captured = board->squares[capturedIndex];
        board->squares[capturedIndex] = PIECE_NONE;
    }

    /* Castling */
    if (move->flags & MOVE_CASTLING)
    {
        // King moves normally
        board->squares[move->to] = movingPiece;
        board->squares[move->from] = PIECE_NONE;

        // Rook move
        if (move->to > move->from)  // king side
        {
            int rookFrom = move->to + 1;
            int rookTo   = move->to - 1;
            board->squares[rookTo] = board->squares[rookFrom];
            board->squares[rookFrom] = PIECE_NONE;
        }
        else                        // queen side
        {
            int rookFrom = move->to - 2;
            int rookTo   = move->to + 1;
            board->squares[rookTo] = board->squares[rookFrom];
            board->squares[rookFrom] = PIECE_NONE;
        }

        return;
    }

    /* =======================
       Normal move
       ======================= */

    board->squares[move->to] = movingPiece;
    board->squares[move->from] = PIECE_NONE;

    /* Promotion */
    if (move->flags & MOVE_PROMOTION)
    {
        int color = (movingPiece > 0) ? 1 : -1;
        board->squares[move->to] = color * move->promotion;
    }
}


void Board_UndoMove(Board *board, const Move *move)
{
    int piece = board->squares[move->to];

    /* Undo promotion */
    if (move->flags & MOVE_PROMOTION)
    {
        int color = (piece > 0) ? 1 : -1;
        piece = color * PAWN;
    }

    /* Castling */
    if (move->flags & MOVE_CASTLING)
    {
        board->squares[move->from] = piece;
        board->squares[move->to] = PIECE_NONE;

        if (move->to > move->from)  // king side
        {
            int rookFrom = move->to + 1;
            int rookTo   = move->to - 1;
            board->squares[rookFrom] = board->squares[rookTo];
            board->squares[rookTo] = PIECE_NONE;
        }
        else                        // queen side
        {
            int rookFrom = move->to - 2;
            int rookTo   = move->to + 1;
            board->squares[rookFrom] = board->squares[rookTo];
            board->squares[rookTo] = PIECE_NONE;
        }

        return;
    }

    /* Normal undo */
    board->squares[move->from] = piece;
    board->squares[move->to] = move->captured;

    /* En passant restore */
    if (move->flags & MOVE_EN_PASSANT)
    {
        int dir = (piece > 0) ? 1 : -1;
        int capturedIndex = move->to + dir * BOARD_FILES;
        board->squares[capturedIndex] = move->captured;
        board->squares[move->to] = PIECE_NONE;
    }
}
