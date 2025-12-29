#ifndef UI_H
#define UI_H

#include "raylib.h"
#include "gamestate.h"

// UI Constants
#define SQUARE_SIZE 80
#define BOARD_OFFSET_X 50
#define BOARD_OFFSET_Y 50
#define WINDOW_WIDTH (BOARD_OFFSET_X * 2 + SQUARE_SIZE * 8)
#define WINDOW_HEIGHT (BOARD_OFFSET_Y * 2 + SQUARE_SIZE * 8 + 100)

// Colors
#define COLOR_LIGHT_SQUARE CLITERAL(Color){240, 217, 181, 255}
#define COLOR_DARK_SQUARE CLITERAL(Color){181, 136, 99, 255}
#define COLOR_SELECTED CLITERAL(Color){255, 255, 0, 150}
#define COLOR_LEGAL_MOVE CLITERAL(Color){100, 255, 100, 100}
#define COLOR_LAST_MOVE CLITERAL(Color){255, 255, 100, 100}
#define COLOR_CHECK CLITERAL(Color){255, 50, 50, 150}

typedef struct
{
    Texture2D pieces[12];  // 6 piece types * 2 colors
    Font font;
    
    int selectedSquare;
    MoveList legalMoves;
    Move lastMove;
    
    Vector2 dragOffset;
    int isDragging;
    int draggedSquare;
    
    bool showLegalMoves;
    bool flipBoard;
    
} UIState;

// Initialize UI resources
void UI_Init(UIState *ui);

// Cleanup UI resources
void UI_Cleanup(UIState *ui);

// Load piece textures
void UI_LoadPieceTextures(UIState *ui);

// Draw the chess board
void UI_DrawBoard(const UIState *ui);

// Draw all pieces on the board
void UI_DrawPieces(const UIState *ui, const GameState *state);

// Draw a single piece
void UI_DrawPiece(const UIState *ui, int pieceValue, int square);

// Draw square highlights (selected, legal moves, last move)
void UI_DrawHighlights(const UIState *ui, const GameState *state);

// Draw board coordinates (a-h, 1-8)
void UI_DrawCoordinates(const UIState *ui);

// Draw game information (side to move, result, etc.)
void UI_DrawGameInfo(const UIState *ui, const GameState *state);

// Convert screen position to board square
int UI_ScreenToSquare(Vector2 position);

// Convert board square to screen position
Vector2 UI_SquareToScreen(int square);

// Handle mouse input and return if a move was made
bool UI_HandleInput(UIState *ui, GameState *state);

// Update UI state based on game state
void UI_Update(UIState *ui, const GameState *state);

// Get piece texture index
int UI_GetPieceTextureIndex(int pieceValue);

// Check if a move is in the legal moves list
Move* UI_FindLegalMove(UIState *ui, int from, int to);

// Handle promotion selection
int UI_PromotionDialog(UIState *ui, int color);

#endif