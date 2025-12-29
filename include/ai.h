#ifndef AI_H
#define AI_H

#include "gamestate.h"
#include "move.h"

// AI difficulty levels
typedef enum {
    AI_EASY = 1,      // Depth 2
    AI_MEDIUM = 2,    // Depth 3
    AI_HARD = 3,      // Depth 4
    AI_EXPERT = 4     // Depth 5
} AIDifficulty;

// AI state structure
typedef struct {
    AIDifficulty difficulty;
    int maxDepth;
    int nodesSearched;
} AIState;

// Initialize AI
void AI_Init(AIState *ai, AIDifficulty difficulty);

// Get the best move for the current position
Move AI_GetBestMove(AIState *ai, GameState *state);

// Evaluate a position (positive = white advantage, negative = black advantage)
int AI_EvaluatePosition(const GameState *state);

// Minimax search with alpha-beta pruning
int AI_Minimax(GameState *state, int depth, int alpha, int beta, int maximizing);

// Piece values for evaluation
#define PAWN_VALUE 100
#define KNIGHT_VALUE 320
#define BISHOP_VALUE 330
#define ROOK_VALUE 500
#define QUEEN_VALUE 900
#define KING_VALUE 20000

// Checkmate/stalemate scores
#define CHECKMATE_SCORE 100000
#define STALEMATE_SCORE 0

#endif