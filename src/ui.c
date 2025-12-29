#include "../include/ui.h"
#include <stdio.h>
#include <stdlib.h>

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
        snprintf(filename, sizeof(filename), "assets/%s-%s.png", 
                 colors[colorIndex], types[typeIndex]);
        
        Image img = LoadImage(filename);
        
        if (img.data != NULL)
        {
            ImageResize(&img, SQUARE_SIZE - 10, SQUARE_SIZE - 10);
            ui->pieces[i] = LoadTextureFromImage(img);
            UnloadImage(img);
        }
        else
        {
            Color fallbackColors[12] = {
                WHITE, LIGHTGRAY, SKYBLUE, PURPLE, PINK, GOLD,
                DARKGRAY, GRAY, BLUE, VIOLET, MAROON, ORANGE
            };
            
            img = GenImageColor(SQUARE_SIZE - 10, SQUARE_SIZE - 10, fallbackColors[i]);
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
    int rank = Board_Rank(square);
    int file = Board_File(square);
    
    return (Vector2){
        BOARD_OFFSET_X + file * SQUARE_SIZE,
        BOARD_OFFSET_Y + rank * SQUARE_SIZE
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
    
    for (int file = 0; file < 8; file++)
    {
        int x = BOARD_OFFSET_X + file * SQUARE_SIZE + SQUARE_SIZE / 2 - 5;
        int y = BOARD_OFFSET_Y + 8 * SQUARE_SIZE + 5;
        char text[2] = {files[file], '\0'};
        DrawText(text, x, y, 20, BLACK);
    }
    
    for (int rank = 0; rank < 8; rank++)
    {
        int x = BOARD_OFFSET_X - 25;
        int y = BOARD_OFFSET_Y + rank * SQUARE_SIZE + SQUARE_SIZE / 2 - 10;
        char text[2] = {ranks[rank], '\0'};
        DrawText(text, x, y, 20, BLACK);
    }
}

void UI_DrawHighlights(const UIState *ui, const GameState *state)
{
    // Highlight last move 
    if (ui->lastMove.from >= 0 && ui->lastMove.to >= 0)
    {
        Vector2 fromPos = UI_SquareToScreen(ui->lastMove.from);
        Vector2 toPos = UI_SquareToScreen(ui->lastMove.to);
        DrawRectangle(fromPos.x, fromPos.y, SQUARE_SIZE, SQUARE_SIZE, COLOR_LAST_MOVE);
        DrawRectangle(toPos.x, toPos.y, SQUARE_SIZE, SQUARE_SIZE, COLOR_LAST_MOVE);
    }
    
    // Highlight selected square 
    if (ui->selectedSquare >= 0)
    {
        Vector2 pos = UI_SquareToScreen(ui->selectedSquare);
        DrawRectangle(pos.x, pos.y, SQUARE_SIZE, SQUARE_SIZE, COLOR_SELECTED);
    }
    
    // Highlight legal moves
    if (ui->showLegalMoves && ui->selectedSquare >= 0)
    {
        for (int i = 0; i < ui->legalMoves.count; i++)
        {
            Move *move = &ui->legalMoves.moves[i];
            if (move->from == ui->selectedSquare)
            {
                Vector2 pos = UI_SquareToScreen(move->to);
                int targetPiece = Board_GetPiece(&state->board, move->to);
                
                // Highlight red if it's a capture or en passant
                if (targetPiece != PIECE_NONE || (move->flags & MOVE_EN_PASSANT))
                {
                    // Using a semi-transparent red for captures
                    DrawRectangle(pos.x, pos.y, SQUARE_SIZE, SQUARE_SIZE, (Color){255, 0, 0, 120});
                }
                else
                {
                    DrawCircle(pos.x + SQUARE_SIZE / 2, pos.y + SQUARE_SIZE / 2, 15, COLOR_LEGAL_MOVE);
                }
            }
        }
    }
    
    if (GameState_IsInCheck(state))
    {
        int kingSquare = GameState_FindKing(state, state->sideToMove);
        if (kingSquare >= 0)
        {
            Vector2 pos = UI_SquareToScreen(kingSquare);
            DrawRectangle(pos.x, pos.y, SQUARE_SIZE, SQUARE_SIZE, COLOR_CHECK);
        }
    }
}

void UI_DrawPiece(const UIState *ui, int pieceValue, int square)
{
    int texIndex = UI_GetPieceTextureIndex(pieceValue);
    if (texIndex < 0) return;
    
    Vector2 pos = UI_SquareToScreen(square);
    DrawTexture(ui->pieces[texIndex], pos.x + 5, pos.y + 5, WHITE);
}

void UI_DrawPieces(const UIState *ui, const GameState *state)
{
    for (int square = 0; square < BOARD_SIZE; square++)
    {
        int piece = Board_GetPiece(&state->board, square);
        if (piece != PIECE_NONE)
        {
            UI_DrawPiece(ui, piece, square);
        }
    }
}

void UI_DrawGameInfo(const UIState *ui, const GameState *state)
{
    int y = BOARD_OFFSET_Y + 8 * SQUARE_SIZE + 40;
    const char *sideStr = (state->sideToMove > 0) ? "White to move" : "Black to move";
    DrawText(sideStr, BOARD_OFFSET_X, y, 20, BLACK);
    
    if (state->result != GAME_ONGOING)
    {
        const char *resultStr[] = { "", "White wins!", "Black wins!", "Stalemate", "50-move rule", "Repetition", "Material" };
        DrawText(resultStr[state->result], BOARD_OFFSET_X, y + 25, 20, RED);
    }
}

Move* UI_FindLegalMove(UIState *ui, int from, int to)
{
    for (int i = 0; i < ui->legalMoves.count; i++)
    {
        Move *move = &ui->legalMoves.moves[i];
        if (move->from == from && move->to == to) return move;
    }
    return NULL;
}

int UI_PromotionDialog(UIState *ui, int color)
{
    int pieces[] = {QUEEN, ROOK, BISHOP, KNIGHT};
    const char *names[] = {"Queen", "Rook", "Bishop", "Knight"};
    int dialogX = (WINDOW_WIDTH - 300) / 2;
    int dialogY = (WINDOW_HEIGHT - 250) / 2;
    
    while (!WindowShouldClose())
    {
        BeginDrawing();
        DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, (Color){0, 0, 0, 150});
        DrawRectangle(dialogX, dialogY, 300, 250, LIGHTGRAY);
        DrawText("Promote to:", dialogX + 20, dialogY + 20, 20, BLACK);
        
        for (int i = 0; i < 4; i++)
        {
            Rectangle button = {dialogX + 20, dialogY + 60 + i * 45, 260, 40};
            if (CheckCollisionPointRec(GetMousePosition(), button) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                EndDrawing();
                return pieces[i];
            }
            DrawRectangleRec(button, GRAY);
            DrawText(names[i], dialogX + 40, button.y + 10, 20, WHITE);
        }
        EndDrawing();
    }
    return QUEEN;
}

void UI_Update(UIState *ui, const GameState *state)
{
    GameState_GenerateLegalMoves((GameState*)state, &ui->legalMoves);
}

bool UI_HandleInput(UIState *ui, GameState *state)
{
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        Vector2 mousePos = GetMousePosition();
        int hoveredSquare = UI_ScreenToSquare(mousePos);
        
        if (hoveredSquare >= 0)
        {
            // 1. If we already have a piece selected, try to move it
            if (ui->selectedSquare >= 0)
            {
                Move *move = UI_FindLegalMove(ui, ui->selectedSquare, hoveredSquare);
                if (move)
                {
                    if (move->flags & MOVE_PROMOTION)
                        move->promotion = UI_PromotionDialog(ui, state->sideToMove);
                    
                    ui->lastMove = *move;
                    GameState_MakeMove(state, move);
                    GameState_UpdateResult(state);
                    UI_Update(ui, state);
                    ui->selectedSquare = -1;
                    return true;
                }
            }
            
            // 2. Otherwise, try to select a piece belonging to the current side
            int piece = Board_GetPiece(&state->board, hoveredSquare);
            if (piece != PIECE_NONE && Board_PieceColor(piece) == state->sideToMove)
            {
                ui->selectedSquare = hoveredSquare;
            }
            else
            {
                ui->selectedSquare = -1;
            }
        }
    }
    return false;
}