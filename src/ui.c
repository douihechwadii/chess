#include "../include/ui.h"
#include <stdio.h>
#include <stdlib.h>

// UI Layout Constants for centering and padding
#define BOARD_PADDING 50
#define CONTAINER_COLOR RAYWHITE
#define TEXT_COLOR_MAIN (Color){ 30, 30, 30, 255 }

void UI_Init(UIState *ui)
{
    ui->selectedSquare = -1;
    ui->showLegalMoves = true;
    ui->lastMove.from = -1;
    ui->lastMove.to = -1;
    
    MoveList_Init(&ui->legalMoves);
    UI_LoadPieceTextures(ui);
    ui->font = GetFontDefault();
}

void UI_Cleanup(UIState *ui)
{
    for (int i = 0; i < 12; i++)
    {
        UnloadTexture(ui->pieces[i]);
    }
}

void UI_LoadPieceTextures(UIState *ui)
{
    const char *colors[2] = {"white", "black"};
    const char *types[6] = {"pawn", "rook", "knight", "bishop", "queen", "king"};
    
    for (int i = 0; i < 12; i++)
    {
        int colorIndex = (i < 6) ? 0 : 1; 
        int typeIndex = i % 6;
        char filename[256];
        snprintf(filename, sizeof(filename), "assets/%s-%s.png", colors[colorIndex], types[typeIndex]);
        
        Image img = LoadImage(filename);
        if (img.data != NULL)
        {
            ImageResize(&img, SQUARE_SIZE - 10, SQUARE_SIZE - 10);
            ui->pieces[i] = LoadTextureFromImage(img);
            UnloadImage(img);
        }
        else
        {
            Color fbColors[12] = { WHITE, LIGHTGRAY, SKYBLUE, PURPLE, PINK, GOLD, DARKGRAY, GRAY, BLUE, VIOLET, MAROON, ORANGE };
            img = GenImageColor(SQUARE_SIZE - 10, SQUARE_SIZE - 10, fbColors[i]);
            ImageDrawRectangleLines(&img, (Rectangle){0, 0, SQUARE_SIZE - 10, SQUARE_SIZE - 10}, 2, BLACK);
            const char *letters = "PRNBQKPRNBQK";
            char text[2] = {letters[i], '\0'};
            ImageDrawText(&img, text, SQUARE_SIZE/2 - 15, SQUARE_SIZE/2 - 15, 30, BLACK);
            ui->pieces[i] = LoadTextureFromImage(img);
            UnloadImage(img);
        }
    }
}

int UI_GetPieceTextureIndex(int pieceValue)
{
    if (pieceValue == PIECE_NONE) return -1;
    int pieceType = Board_PieceType(pieceValue);
    int color = Board_PieceColor(pieceValue);
    int baseIndex = (pieceType - 1);
    if (color < 0) baseIndex += 6;
    return baseIndex;
}

Vector2 UI_SquareToScreen(int square)
{
    return (Vector2){
        BOARD_OFFSET_X + Board_File(square) * SQUARE_SIZE,
        BOARD_OFFSET_Y + Board_Rank(square) * SQUARE_SIZE
    };
}

int UI_ScreenToSquare(Vector2 position)
{
    int file = (int)((position.x - BOARD_OFFSET_X) / SQUARE_SIZE);
    int rank = (int)((position.y - BOARD_OFFSET_Y) / SQUARE_SIZE);
    if (file < 0 || file >= 8 || rank < 0 || rank >= 8) return -1;
    return Board_Index(rank, file);
}

void UI_DrawBoard(const UIState *ui)
{
    // 1. Draw the "White Box" container with equal padding on all sides
    Rectangle container = {
        BOARD_OFFSET_X - BOARD_PADDING,
        BOARD_OFFSET_Y - BOARD_PADDING,
        (SQUARE_SIZE * 8) + (BOARD_PADDING * 2),
        (SQUARE_SIZE * 8) + (BOARD_PADDING * 2)
    };
    DrawRectangleRec(container, CONTAINER_COLOR);
    DrawRectangleLinesEx(container, 2, LIGHTGRAY);

    // 2. Draw the board squares
    for (int rank = 0; rank < 8; rank++)
    {
        for (int file = 0; file < 8; file++)
        {
            Vector2 pos = { BOARD_OFFSET_X + file * SQUARE_SIZE, BOARD_OFFSET_Y + rank * SQUARE_SIZE };
            Color color = ((rank + file) % 2 == 0) ? COLOR_LIGHT_SQUARE : COLOR_DARK_SQUARE;
            DrawRectangle(pos.x, pos.y, SQUARE_SIZE, SQUARE_SIZE, color);
        }
    }
}

void UI_DrawCoordinates(const UIState *ui)
{
    const char *files = "abcdefgh";
    const char *ranks = "87654321";
    
    for (int i = 0; i < 8; i++)
    {
        // Files (placed inside the bottom padding area)
        DrawText(TextFormat("%c", files[i]), BOARD_OFFSET_X + i * SQUARE_SIZE + (SQUARE_SIZE/2 - 5), BOARD_OFFSET_Y + (8 * SQUARE_SIZE) + 12, 18, DARKGRAY);
        // Ranks (placed inside the left padding area)
        DrawText(TextFormat("%c", ranks[i]), BOARD_OFFSET_X - 30, BOARD_OFFSET_Y + i * SQUARE_SIZE + (SQUARE_SIZE/2 - 10), 18, DARKGRAY);
    }
}

void UI_DrawHighlights(const UIState *ui, const GameState *state)
{
    // Highlight last move
    if (ui->lastMove.from >= 0)
    {
        DrawRectangleRec((Rectangle){UI_SquareToScreen(ui->lastMove.from).x, UI_SquareToScreen(ui->lastMove.from).y, SQUARE_SIZE, SQUARE_SIZE}, COLOR_LAST_MOVE);
        DrawRectangleRec((Rectangle){UI_SquareToScreen(ui->lastMove.to).x, UI_SquareToScreen(ui->lastMove.to).y, SQUARE_SIZE, SQUARE_SIZE}, COLOR_LAST_MOVE);
    }
    
    // Selection and Legal Move logic
    if (ui->selectedSquare >= 0)
    {
        Vector2 pos = UI_SquareToScreen(ui->selectedSquare);
        DrawRectangle(pos.x, pos.y, SQUARE_SIZE, SQUARE_SIZE, COLOR_SELECTED);

        for (int i = 0; i < ui->legalMoves.count; i++)
        {
            Move *move = &ui->legalMoves.moves[i];
            if (move->from == ui->selectedSquare)
            {
                Vector2 mPos = UI_SquareToScreen(move->to);
                int target = Board_GetPiece(&state->board, move->to);
                
                // Highlight red if it's a capture or en passant
                if (target != PIECE_NONE || (move->flags & MOVE_EN_PASSANT))
                {
                    DrawRectangle(mPos.x, mPos.y, SQUARE_SIZE, SQUARE_SIZE, (Color){255, 0, 0, 120});
                }
                else
                {
                    DrawCircle(mPos.x + SQUARE_SIZE/2, mPos.y + SQUARE_SIZE/2, 10, (Color){ 0, 0, 0, 40 });
                }
            }
        }
    }
    
    if (GameState_IsInCheck(state))
    {
        int kSq = GameState_FindKing(state, state->sideToMove);
        if (kSq >= 0) DrawRectangleRec((Rectangle){UI_SquareToScreen(kSq).x, UI_SquareToScreen(kSq).y, SQUARE_SIZE, SQUARE_SIZE}, COLOR_CHECK);
    }
}

void UI_DrawPieces(const UIState *ui, const GameState *state)
{
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        int p = Board_GetPiece(&state->board, i);
        if (p != PIECE_NONE)
        {
            int texIdx = UI_GetPieceTextureIndex(p);
            if (texIdx >= 0) DrawTexture(ui->pieces[texIdx], UI_SquareToScreen(i).x + 5, UI_SquareToScreen(i).y + 5, WHITE);
        }
    }
}

void UI_DrawGameInfo(const UIState *ui, const GameState *state)
{
    // Position text below the board container
    int textY = BOARD_OFFSET_Y + (SQUARE_SIZE * 8) + BOARD_PADDING + 15;
    int centerX = BOARD_OFFSET_X;

    // 1. Current Turn
    const char *turnText = (state->sideToMove > 0) ? "WHITE TO MOVE" : "BLACK TO MOVE";
    DrawText(turnText, centerX, textY, 22, TEXT_COLOR_MAIN);

    // 2. Game Result or Check status
    if (state->result != GAME_ONGOING)
    {
        const char *results[] = { "", "WHITE WINS!", "BLACK WINS!", "STALEMATE", "DRAW (50-MOVE)", "REPETITION", "MATERIAL" };
        DrawText(results[state->result], centerX + 250, textY, 22, MAROON);
    }
    else if (GameState_IsInCheck(state))
    {
        DrawText("CHECK!", centerX + 250, textY, 22, RED);
    }
}

Move* UI_FindLegalMove(UIState *ui, int from, int to)
{
    for (int i = 0; i < ui->legalMoves.count; i++)
    {
        if (ui->legalMoves.moves[i].from == from && ui->legalMoves.moves[i].to == to)
            return &ui->legalMoves.moves[i];
    }
    return NULL;
}

int UI_PromotionDialog(UIState *ui, int color)
{
    Rectangle box = {(WINDOW_WIDTH - 300)/2, (WINDOW_HEIGHT - 250)/2, 300, 250};
    int pieces[] = {QUEEN, ROOK, BISHOP, KNIGHT};
    const char *names[] = {"Queen", "Rook", "Bishop", "Knight"};
    
    while (!WindowShouldClose())
    {
        BeginDrawing();
        DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, (Color){0, 0, 0, 150});
        DrawRectangleRec(box, RAYWHITE);
        DrawRectangleLinesEx(box, 2, BLACK);
        DrawText("Promote to:", box.x + 20, box.y + 20, 20, BLACK);
        
        for (int i = 0; i < 4; i++)
        {
            Rectangle btn = {box.x + 20, box.y + 60 + i * 45, 260, 40};
            if (CheckCollisionPointRec(GetMousePosition(), btn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) { EndDrawing(); return pieces[i]; }
            DrawRectangleRec(btn, LIGHTGRAY);
            DrawText(names[i], btn.x + 20, btn.y + 10, 20, BLACK);
        }
        EndDrawing();
    }
    return QUEEN;
}

void UI_Update(UIState *ui, const GameState *state) { GameState_GenerateLegalMoves((GameState*)state, &ui->legalMoves); }

bool UI_HandleInput(UIState *ui, GameState *state)
{
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        int hovered = UI_ScreenToSquare(GetMousePosition());
        if (hovered >= 0)
        {
            if (ui->selectedSquare >= 0)
            {
                Move *m = UI_FindLegalMove(ui, ui->selectedSquare, hovered);
                if (m)
                {
                    if (m->flags & MOVE_PROMOTION) m->promotion = UI_PromotionDialog(ui, state->sideToMove);
                    ui->lastMove = *m;
                    GameState_MakeMove(state, m);
                    GameState_UpdateResult(state);
                    UI_Update(ui, state);
                    ui->selectedSquare = -1;
                    return true;
                }
            }
            int p = Board_GetPiece(&state->board, hovered);
            if (p != PIECE_NONE && Board_PieceColor(p) == state->sideToMove) ui->selectedSquare = hovered;
            else ui->selectedSquare = -1;
        }
    }
    return false;
}