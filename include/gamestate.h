#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "board.h"
#include "move.h"
#include "movegen.h"

// Castling rights bit flags
#define CASTLING_NONE        0
#define CASTLING_WHITE_KING  (1 << 0)
#define CASTLING_WHITE_QUEEN (1 << 1)
#define CASTLING_BLACK_KING  (1 << 2)
#define CASTLING_BLACK_QUEEN (1 << 3)
#define CASTLING_WHITE (CASTLING_WHITE_KING | CASTLING_WHITE_QUEEN)
#define CASTLING_BLACK (CASTLING_BLACK_KING | CASTLING_BLACK_QUEEN)
#define CASTLING_ALL   (CASTLING_WHITE | CASTLING_BLACK)

// Game result
typedef enum
{
    GAME_ONGOING,
    GAME_WHITE_WINS,
    GAME_BLACK_WINS,
    GAME_DRAW_STALEMATE,
    GAME_DRAW_FIFTY_MOVE,
    GAME_DRAW_THREEFOLD,
    GAME_DRAW_INSUFFICIENT
} GameResult;

typedef struct
{
    Board board;
    
    int sideToMove;          // 1 for white, -1 for black
    int castlingRights;      // Bit flags for castling availability 
    int enPassantSquare;     // Target square for en passant, or -1 
    int halfmoveClock;       // Moves since last capture or pawn move 
    int fullmoveNumber;      // Starts at 1, incremented after black's move 
    
    GameResult result;
} GameState;

// Initialize game state from FEN string 
void GameState_InitFromFEN(GameState *state, const char *fen);

// Initialize to starting position 
void GameState_Init(GameState *state);

// Make a move and update game state 
void GameState_MakeMove(GameState *state, Move *move);

// Unmake a move (for search/analysis) 
void GameState_UnmakeMove(GameState *state, const Move *move);

// Check if a square is attacked by a given color 
int GameState_IsSquareAttacked(const GameState *state, int square, int attackerColor);

// Check if the current side to move is in check 
int GameState_IsInCheck(const GameState *state);

// Find the king of a given color 
int GameState_FindKing(const GameState *state, int color);

// Generate all legal moves (filters out moves that leave king in check) 
void GameState_GenerateLegalMoves(GameState *state, MoveList *list);

// Check if a move is legal 
int GameState_IsMoveLegal(GameState *state, const Move *move);

// Update game result (checkmate, stalemate, etc.) 
void GameState_UpdateResult(GameState *state);

// Check for draw conditions 
int GameState_IsDrawByFiftyMove(const GameState *state);
int GameState_IsDrawByInsufficientMaterial(const GameState *state);

// Print game state information 
void GameState_Print(const GameState *state);

// Helper to get castling rights string (for FEN) 
void GameState_GetCastlingString(const GameState *state, char *buffer);

#endif