#include "../include/ai.h"
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>

// Piece-square tables for positional evaluation
static const int pawnTable[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
     5,  5, 10, 25, 25, 10,  5,  5,
     0,  0,  0, 20, 20,  0,  0,  0,
     5, -5,-10,  0,  0,-10, -5,  5,
     5, 10, 10,-20,-20, 10, 10,  5,
     0,  0,  0,  0,  0,  0,  0,  0
};

static const int knightTable[64] = {
   -50,-40,-30,-30,-30,-30,-40,-50,
   -40,-20,  0,  0,  0,  0,-20,-40,
   -30,  0, 10, 15, 15, 10,  0,-30,
   -30,  5, 15, 20, 20, 15,  5,-30,
   -30,  0, 15, 20, 20, 15,  0,-30,
   -30,  5, 10, 15, 15, 10,  5,-30,
   -40,-20,  0,  5,  5,  0,-20,-40,
   -50,-40,-30,-30,-30,-30,-40,-50
};

static const int bishopTable[64] = {
   -20,-10,-10,-10,-10,-10,-10,-20,
   -10,  0,  0,  0,  0,  0,  0,-10,
   -10,  0,  5, 10, 10,  5,  0,-10,
   -10,  5,  5, 10, 10,  5,  5,-10,
   -10,  0, 10, 10, 10, 10,  0,-10,
   -10, 10, 10, 10, 10, 10, 10,-10,
   -10,  5,  0,  0,  0,  0,  5,-10,
   -20,-10,-10,-10,-10,-10,-10,-20
};

static const int rookTable[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,
     5, 10, 10, 10, 10, 10, 10,  5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
     0,  0,  0,  5,  5,  0,  0,  0
};

static const int queenTable[64] = {
   -20,-10,-10, -5, -5,-10,-10,-20,
   -10,  0,  0,  0,  0,  0,  0,-10,
   -10,  0,  5,  5,  5,  5,  0,-10,
    -5,  0,  5,  5,  5,  5,  0, -5,
     0,  0,  5,  5,  5,  5,  0, -5,
   -10,  5,  5,  5,  5,  5,  0,-10,
   -10,  0,  5,  0,  0,  0,  0,-10,
   -20,-10,-10, -5, -5,-10,-10,-20
};

static const int kingMiddleGameTable[64] = {
   -30,-40,-40,-50,-50,-40,-40,-30,
   -30,-40,-40,-50,-50,-40,-40,-30,
   -30,-40,-40,-50,-50,-40,-40,-30,
   -30,-40,-40,-50,-50,-40,-40,-30,
   -20,-30,-30,-40,-40,-30,-30,-20,
   -10,-20,-20,-20,-20,-20,-20,-10,
    20, 20,  0,  0,  0,  0, 20, 20,
    20, 30, 10,  0,  0, 10, 30, 20
};

void AI_Init(AIState *ai, AIDifficulty difficulty)
{
    ai->difficulty = difficulty;
    ai->maxDepth = difficulty + 1;  // Easy=2, Medium=3, Hard=4, Expert=5
    ai->nodesSearched = 0;
}

static int GetPieceValue(int pieceType)
{
    switch (pieceType) {
        case PAWN: return PAWN_VALUE;
        case KNIGHT: return KNIGHT_VALUE;
        case BISHOP: return BISHOP_VALUE;
        case ROOK: return ROOK_VALUE;
        case QUEEN: return QUEEN_VALUE;
        case KING: return KING_VALUE;
        default: return 0;
    }
}

static int GetPositionalValue(int pieceType, int square, int color)
{
    // Flip square for black pieces
    if (color < 0) {
        int rank = Board_Rank(square);
        int file = Board_File(square);
        square = Board_Index(7 - rank, file);
    }
    
    switch (pieceType) {
        case PAWN: return pawnTable[square];
        case KNIGHT: return knightTable[square];
        case BISHOP: return bishopTable[square];
        case ROOK: return rookTable[square];
        case QUEEN: return queenTable[square];
        case KING: return kingMiddleGameTable[square];
        default: return 0;
    }
}

int AI_EvaluatePosition(const GameState *state)
{
    int score = 0;
    
    // Material and positional evaluation
    for (int square = 0; square < BOARD_SIZE; square++) {
        int piece = Board_GetPiece(&state->board, square);
        if (piece == PIECE_NONE) continue;
        
        int pieceType = Board_PieceType(piece);
        int color = Board_PieceColor(piece);
        
        int materialValue = GetPieceValue(pieceType);
        int positionalValue = GetPositionalValue(pieceType, square, color);
        
        score += color * (materialValue + positionalValue);
    }
    
    // Bonus for castling rights
    if (state->castlingRights & CASTLING_WHITE_KING) score += 30;
    if (state->castlingRights & CASTLING_WHITE_QUEEN) score += 30;
    if (state->castlingRights & CASTLING_BLACK_KING) score -= 30;
    if (state->castlingRights & CASTLING_BLACK_QUEEN) score -= 30;
    
    // Small bonus for side to move (tempo)
    score += state->sideToMove * 10;
    
    return score;
}

int AI_Minimax(GameState *state, int depth, int alpha, int beta, int maximizing)
{
    if (depth == 0) {
        return AI_EvaluatePosition(state);
    }
    
    MoveList legalMoves;
    GameState_GenerateLegalMoves(state, &legalMoves);
    
    // Terminal node evaluation
    if (legalMoves.count == 0) {
        if (GameState_IsInCheck(state)) {
            // Checkmate - return score based on depth to prefer faster mates
            return maximizing ? -CHECKMATE_SCORE + depth : CHECKMATE_SCORE - depth;
        } else {
            // Stalemate
            return STALEMATE_SCORE;
        }
    }
    
    if (maximizing) {
        int maxEval = INT_MIN;
        
        for (int i = 0; i < legalMoves.count; i++) {
            Move *move = &legalMoves.moves[i];
            
            // Save state
            GameState tempState = *state;
            
            // Make move
            GameState_MakeMove(state, move);
            
            // Recursive evaluation
            int eval = AI_Minimax(state, depth - 1, alpha, beta, 0);
            
            // Restore state
            *state = tempState;
            
            maxEval = (eval > maxEval) ? eval : maxEval;
            alpha = (alpha > eval) ? alpha : eval;
            
            if (beta <= alpha) {
                break;  // Beta cutoff
            }
        }
        
        return maxEval;
    } else {
        int minEval = INT_MAX;
        
        for (int i = 0; i < legalMoves.count; i++) {
            Move *move = &legalMoves.moves[i];
            
            // Save state
            GameState tempState = *state;
            
            // Make move
            GameState_MakeMove(state, move);
            
            // Recursive evaluation
            int eval = AI_Minimax(state, depth - 1, alpha, beta, 1);
            
            // Restore state
            *state = tempState;
            
            minEval = (eval < minEval) ? eval : minEval;
            beta = (beta < eval) ? beta : eval;
            
            if (beta <= alpha) {
                break;  // Alpha cutoff
            }
        }
        
        return minEval;
    }
}

Move AI_GetBestMove(AIState *ai, GameState *state)
{
    MoveList legalMoves;
    GameState_GenerateLegalMoves(state, &legalMoves);
    
    if (legalMoves.count == 0) {
        Move nullMove = {-1, -1, 0, 0, 0};
        return nullMove;
    }
    
    ai->nodesSearched = 0;
    
    Move bestMove = legalMoves.moves[0];
    int bestScore = (state->sideToMove > 0) ? INT_MIN : INT_MAX;
    
    printf("AI thinking (depth %d)...\n", ai->maxDepth);
    
    for (int i = 0; i < legalMoves.count; i++) {
        Move *move = &legalMoves.moves[i];
        
        // Save state
        GameState tempState = *state;
        
        // Make move
        GameState_MakeMove(state, move);
        
        // Evaluate position
        int score = AI_Minimax(state, ai->maxDepth - 1, INT_MIN, INT_MAX, 
                               state->sideToMove > 0);
        
        // Restore state
        *state = tempState;
        
        // Update best move
        if (state->sideToMove > 0) {
            if (score > bestScore) {
                bestScore = score;
                bestMove = *move;
            }
        } else {
            if (score < bestScore) {
                bestScore = score;
                bestMove = *move;
            }
        }
        
        ai->nodesSearched++;
    }
    
    printf("Best move found with score: %d (searched %d positions)\n", 
           bestScore, ai->nodesSearched);
    
    return bestMove;
}