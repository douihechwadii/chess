#include "../include/board.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

static int FenCharToPieceValue(char c)
{
    switch (c)
    {
    case 'p':
        return -PAWN;
    case 'r':
        return -ROOK;
    case 'n':
        return -KNIGHT;
    case 'b':
        return -BISHOP;
    case 'q':
        return -QUEEN;
    case 'k':
        return -KING;

    case 'P':
        return PAWN;
    case 'R':
        return ROOK;
    case 'N':
        return KNIGHT;
    case 'B':
        return BISHOP;
    case 'Q':
        return QUEEN;
    case 'K':
        return KING;

    default:
        return PIECE_NONE;
    }
}

void Board_Clear(Board *board)
{
    memset(board->squares, 0, sizeof(board->squares));
}

void InitFromFEN(Board *board, const char *fen)
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
    return value < 0 ? -value : value;
}

int Board_PieceColor(int value)
{
    if (value > 0)
        return 1;
    if (value < 0)
        return -1;
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
            case PAWN:
                c = 'p';
                break;
            case ROOK:
                c = 'r';
                break;
            case KNIGHT:
                c = 'n';
                break;
            case BISHOP:
                c = 'b';
                break;
            case QUEEN:
                c = 'q';
                break;
            case KING:
                c = 'k';
                break;
            default:
                c = '.';
                break;
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
