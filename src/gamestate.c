#include "../include/gamestate.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

void GameState_Init(GameState *state)
{
    GameState_InitFromFEN(state, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

void GameState_InitFromFEN(GameState *state, const char *fen)
{
    InitFromFEN(&state->board, fen);
    
    // Find the space after piece placement 
    const char *ptr = fen;
    while (*ptr && *ptr != ' ') ptr++;
    if (*ptr == ' ') ptr++;
    
    // Side to move 
    state->sideToMove = (*ptr == 'w') ? 1 : -1;
    ptr += 2;
    
    // Castling rights 
    state->castlingRights = CASTLING_NONE;
    while (*ptr && *ptr != ' ')
    {
        switch (*ptr)
        {
            case 'K': state->castlingRights |= CASTLING_WHITE_KING; break;
            case 'Q': state->castlingRights |= CASTLING_WHITE_QUEEN; break;
            case 'k': state->castlingRights |= CASTLING_BLACK_KING; break;
            case 'q': state->castlingRights |= CASTLING_BLACK_QUEEN; break;
        }
        ptr++;
    }
    if (*ptr == ' ') ptr++;
    
    // En passant square 
    state->enPassantSquare = -1;
    if (*ptr != '-')
    {
        int file = ptr[0] - 'a';
        int rank = 8 - (ptr[1] - '0');
        state->enPassantSquare = Board_Index(rank, file);
        ptr += 2;
    }
    else
    {
        ptr++;
    }
    if (*ptr == ' ') ptr++;
    
    // Halfmove clock 
    state->halfmoveClock = atoi(ptr);
    while (*ptr && *ptr != ' ') ptr++;
    if (*ptr == ' ') ptr++;
    
    // Fullmove number 
    state->fullmoveNumber = atoi(ptr);
    
    state->result = GAME_ONGOING;
}

int GameState_FindKing(const GameState *state, int color)
{
    int kingValue = color * KING;
    
    for (int square = 0; square < BOARD_SIZE; square++)
    {
        if (Board_GetPiece(&state->board, square) == kingValue)
        {
            return square;
        }
    }
    
    return -1;
}

int GameState_IsSquareAttacked(const GameState *state, int square, int attackerColor)
{
    int rank = Board_Rank(square);
    int file = Board_File(square);
    
    // Check for pawn attacks 
    int pawnDir = (attackerColor > 0) ? -1 : 1;
    int pawnRank = rank - pawnDir;
    
    for (int pawnFile = file - 1; pawnFile <= file + 1; pawnFile += 2)
    {
        if (MoveGen_IsValidSquare(pawnRank, pawnFile))
        {
            int pawnSquare = Board_Index(pawnRank, pawnFile);
            int piece = Board_GetPiece(&state->board, pawnSquare);
            if (piece == attackerColor * PAWN)
            {
                return 1;
            }
        }
    }
    
    // Check for knight attacks 
    int knightMoves[8][2] = {
        {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
        {1, -2}, {1, 2}, {2, -1}, {2, 1}
    };
    
    for (int i = 0; i < 8; i++)
    {
        int newRank = rank + knightMoves[i][0];
        int newFile = file + knightMoves[i][1];
        
        if (MoveGen_IsValidSquare(newRank, newFile))
        {
            int knightSquare = Board_Index(newRank, newFile);
            int piece = Board_GetPiece(&state->board, knightSquare);
            if (piece == attackerColor * KNIGHT)
            {
                return 1;
            }
        }
    }
    
    // Check for sliding piece attacks (bishop, rook, queen) 
    int directions[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1},
        {0, -1},           {0, 1},
        {1, -1},  {1, 0},  {1, 1}
    };
    
    for (int dir = 0; dir < 8; dir++)
    {
        int isDiagonal = (directions[dir][0] != 0 && directions[dir][1] != 0);
        
        for (int dist = 1; dist < 8; dist++)
        {
            int newRank = rank + dist * directions[dir][0];
            int newFile = file + dist * directions[dir][1];
            
            if (!MoveGen_IsValidSquare(newRank, newFile))
                break;
            
            int targetSquare = Board_Index(newRank, newFile);
            int piece = Board_GetPiece(&state->board, targetSquare);
            
            if (piece != PIECE_NONE)
            {
                if (Board_PieceColor(piece) == attackerColor)
                {
                    int pieceType = Board_PieceType(piece);
                    
                    if (pieceType == QUEEN)
                    {
                        return 1;
                    }
                    else if (isDiagonal && pieceType == BISHOP)
                    {
                        return 1;
                    }
                    else if (!isDiagonal && pieceType == ROOK)
                    {
                        return 1;
                    }
                    else if (dist == 1 && pieceType == KING)
                    {
                        return 1;
                    }
                }
                break;
            }
        }
    }
    
    return 0;
}

int GameState_IsInCheck(const GameState *state)
{
    int kingSquare = GameState_FindKing(state, state->sideToMove);
    if (kingSquare < 0)
        return 0;
    
    return GameState_IsSquareAttacked(state, kingSquare, -state->sideToMove);
}

int GameState_IsMoveLegal(GameState *state, const Move *move)
{
    // Make the move 
    Board tempBoard = state->board;
    int tempEnPassant = state->enPassantSquare;
    int tempCastling = state->castlingRights;
    
    Board_DoMove(&state->board, (Move*)move);
    
    // Check if our king is in check after the move 
    int kingSquare = GameState_FindKing(state, state->sideToMove);
    int isLegal = (kingSquare >= 0) && 
                  !GameState_IsSquareAttacked(state, kingSquare, -state->sideToMove);
    
    // Restore board 
    state->board = tempBoard;
    state->enPassantSquare = tempEnPassant;
    state->castlingRights = tempCastling;
    
    return isLegal;
}

void GameState_GenerateLegalMoves(GameState *state, MoveList *list)
{
    MoveList pseudoLegal;
    // FIXED: Pass enPassantSquare and castlingRights to MoveGen_GenerateAllMoves
    MoveGen_GenerateAllMoves(&state->board, &pseudoLegal, state->sideToMove, state->enPassantSquare, state->castlingRights);
    
    MoveList_Init(list);
    
    for (int i = 0; i < pseudoLegal.count; i++)
    {
        Move *move = &pseudoLegal.moves[i];
        
        // Check castling validity 
        if (move->flags & MOVE_CASTLING)
        {
            int kingSquare = move->from;
            int targetSquare = move->to;
            
            // Can't castle out of check 
            if (GameState_IsInCheck(state))
                continue;
            
            // Can't castle through check 
            int step = (targetSquare > kingSquare) ? 1 : -1;
            int throughSquare = kingSquare + step;
            
            if (GameState_IsSquareAttacked(state, throughSquare, -state->sideToMove))
                continue;
            
            // Can't castle into check (will be caught by general legality check) 
        }
        
        if (GameState_IsMoveLegal(state, move))
        {
            list->moves[list->count++] = *move;
        }
    }
}

void GameState_MakeMove(GameState *state, Move *move)
{
    int movingPiece = Board_GetPiece(&state->board, move->from);
    int pieceType = Board_PieceType(movingPiece);
    int fromRank = Board_Rank(move->from);
    int toRank = Board_Rank(move->to);
    
    // Update halfmove clock 
    if (pieceType == PAWN || move->flags & MOVE_CAPTURE)
    {
        state->halfmoveClock = 0;
    }
    else
    {
        state->halfmoveClock++;
    }
    
    // Clear en passant square 
    state->enPassantSquare = -1;
    
    // Set en passant square for double pawn push 
    if (pieceType == PAWN && abs(toRank - fromRank) == 2)
    {
        state->enPassantSquare = Board_Index((fromRank + toRank) / 2, Board_File(move->from));
    }
    
    // Update castling rights 
    if (pieceType == KING)
    {
        if (state->sideToMove > 0)
        {
            state->castlingRights &= ~CASTLING_WHITE;
        }
        else
        {
            state->castlingRights &= ~CASTLING_BLACK;
        }
    }
    
    if (pieceType == ROOK)
    {
        if (state->sideToMove > 0)
        {
            if (move->from == Board_Index(7, 0))
                state->castlingRights &= ~CASTLING_WHITE_QUEEN;
            else if (move->from == Board_Index(7, 7))
                state->castlingRights &= ~CASTLING_WHITE_KING;
        }
        else
        {
            if (move->from == Board_Index(0, 0))
                state->castlingRights &= ~CASTLING_BLACK_QUEEN;
            else if (move->from == Board_Index(0, 7))
                state->castlingRights &= ~CASTLING_BLACK_KING;
        }
    }
    
    // If a rook is captured, remove castling rights 
    if (move->flags & MOVE_CAPTURE)
    {
        if (move->to == Board_Index(7, 0))
            state->castlingRights &= ~CASTLING_WHITE_QUEEN;
        else if (move->to == Board_Index(7, 7))
            state->castlingRights &= ~CASTLING_WHITE_KING;
        else if (move->to == Board_Index(0, 0))
            state->castlingRights &= ~CASTLING_BLACK_QUEEN;
        else if (move->to == Board_Index(0, 7))
            state->castlingRights &= ~CASTLING_BLACK_KING;
    }
    
    // Execute the move 
    Board_DoMove(&state->board, move);
    
    // Switch sides 
    state->sideToMove = -state->sideToMove;
    
    // Increment fullmove number after black's move 
    if (state->sideToMove > 0)
    {
        state->fullmoveNumber++;
    }
}

void GameState_UnmakeMove(GameState *state, const Move *move)
{
    // Switch sides back 
    state->sideToMove = -state->sideToMove;
    
    // Decrement fullmove number if we're unmaking black's move 
    if (state->sideToMove < 0)
    {
        state->fullmoveNumber--;
    }
    
    // Undo the move 
    Board_UndoMove(&state->board, move);
    
    // Note: This simple unmake doesn't restore castling rights, en passant, 
    //   or halfmove clock. For a full implementation, i'd need to store
    //   these in a move history stack 
}

void GameState_UpdateResult(GameState *state)
{
    MoveList legalMoves;
    GameState_GenerateLegalMoves(state, &legalMoves);
    
    if (legalMoves.count == 0)
    {
        if (GameState_IsInCheck(state))
        {
            // Checkmate 
            state->result = (state->sideToMove > 0) ? GAME_BLACK_WINS : GAME_WHITE_WINS;
        }
        else
        {
            // Stalemate 
            state->result = GAME_DRAW_STALEMATE;
        }
    }
    else if (GameState_IsDrawByFiftyMove(state))
    {
        state->result = GAME_DRAW_FIFTY_MOVE;
    }
    else if (GameState_IsDrawByInsufficientMaterial(state))
    {
        state->result = GAME_DRAW_INSUFFICIENT;
    }
    else
    {
        state->result = GAME_ONGOING;
    }
}

int GameState_IsDrawByFiftyMove(const GameState *state)
{
    return state->halfmoveClock >= 100;
}

int GameState_IsDrawByInsufficientMaterial(const GameState *state)
{
    int whitePieces = 0, blackPieces = 0;
    int whiteBishops = 0, blackBishops = 0;
    int whiteKnights = 0, blackKnights = 0;
    
    for (int square = 0; square < BOARD_SIZE; square++)
    {
        int piece = Board_GetPiece(&state->board, square);
        if (piece == PIECE_NONE)
            continue;
        
        int pieceType = Board_PieceType(piece);
        int color = Board_PieceColor(piece);
        
        if (color > 0)
            whitePieces++;
        else
            blackPieces++;
        
        // Pawns, rooks, queens mean sufficient material 
        if (pieceType == PAWN || pieceType == ROOK || pieceType == QUEEN)
            return 0;
        
        if (pieceType == BISHOP)
        {
            if (color > 0)
                whiteBishops++;
            else
                blackBishops++;
        }
        else if (pieceType == KNIGHT)
        {
            if (color > 0)
                whiteKnights++;
            else
                blackKnights++;
        }
    }
    
    // King vs King 
    if (whitePieces == 1 && blackPieces == 1)
        return 1;
    
    // King and minor piece vs King 
    if ((whitePieces == 2 && blackPieces == 1 && (whiteBishops == 1 || whiteKnights == 1)) ||
        (whitePieces == 1 && blackPieces == 2 && (blackBishops == 1 || blackKnights == 1)))
        return 1;
    
    // King and Bishop vs King and Bishop (same color squares) 
    // This is simplified - would need to check square colors 
    
    return 0;
}

void GameState_GetCastlingString(const GameState *state, char *buffer)
{
    int pos = 0;
    
    if (state->castlingRights & CASTLING_WHITE_KING)
        buffer[pos++] = 'K';
    if (state->castlingRights & CASTLING_WHITE_QUEEN)
        buffer[pos++] = 'Q';
    if (state->castlingRights & CASTLING_BLACK_KING)
        buffer[pos++] = 'k';
    if (state->castlingRights & CASTLING_BLACK_QUEEN)
        buffer[pos++] = 'q';
    
    if (pos == 0)
        buffer[pos++] = '-';
    
    buffer[pos] = '\0';
}

void GameState_Print(const GameState *state)
{
    Board_DebugPrint(&state->board);
    
    printf("Side to move: %s\n", (state->sideToMove > 0) ? "White" : "Black");
    
    char castling[8];
    GameState_GetCastlingString(state, castling);
    printf("Castling rights: %s\n", castling);
    
    if (state->enPassantSquare >= 0)
    {
        char files[] = "abcdefgh";
        int rank = Board_Rank(state->enPassantSquare);
        int file = Board_File(state->enPassantSquare);
        printf("En passant: %c%d\n", files[file], 8 - rank);
    }
    else
    {
        printf("En passant: -\n");
    }
    
    printf("Halfmove clock: %d\n", state->halfmoveClock);
    printf("Fullmove number: %d\n", state->fullmoveNumber);
    
    if (GameState_IsInCheck(state))
    {
        printf("*** IN CHECK ***\n");
    }
    
    const char *resultStr[] = {
        "Ongoing",
        "White wins by checkmate",
        "Black wins by checkmate",
        "Draw by stalemate",
        "Draw by fifty-move rule",
        "Draw by threefold repetition",
        "Draw by insufficient material"
    };
    printf("Game result: %s\n", resultStr[state->result]);
    
    printf("\n");
}