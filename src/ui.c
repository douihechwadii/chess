#include "../include/ui.h"
#include <stdio.h>
#include <stdlib.h>

void UI_Init(UIState *ui)
{
    ui->selectedSquare = -1;
    ui->isDragging = 0;
    ui->draggedSquare = -1;
    ui->showLegalMoves = true;
    ui->flipBoard = false;
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
    /* Load actual piece images from assets folder */
    /* Naming pattern: color-type.png (e.g., white-pawn.png, black-bishop.png) */
    
    const char *colors[2] = {"white", "black"};
    const char *types[6] = {"pawn", "rook", "knight", "bishop", "queen", "king"};
    
    for (int i = 0; i < 12; i++)
    {
        int colorIndex = (i < 6) ? 0 : 1;  /* 0-5: white, 6-11: black */
        int typeIndex = i % 6;
        
        /* Build the filename: assets/white-pawn.png */
        char filename[256];
        snprintf(filename, sizeof(filename), "assets/%s-%s.png", 
                 colors[colorIndex], types[typeIndex]);
        
        /* Try to load the image */
        Image img = LoadImage(filename);
        
        if (img.data != NULL)
        {
            /* Resize image to fit the square (with some padding) */
            ImageResize(&img, SQUARE_SIZE - 10, SQUARE_SIZE - 10);
            ui->pieces[i] = LoadTextureFromImage(img);
            UnloadImage(img);
            printf("Loaded: %s\n", filename);
        }
        else
        {
            /* Fallback: Create a colored square with letter if image not found */
            printf("Warning: Could not load %s, using fallback\n", filename);
            
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
    if (pieceValue == PIECE_NONE)
        return -1;
    
    int pieceType = Board_PieceType(pieceValue);
    int color = Board_PieceColor(pieceValue);
    
    int baseIndex = (pieceType - 1);
    if (color < 0)
        baseIndex += 6;
    
    return baseIndex;
}

Vector2 UI_SquareToScreen(int square, bool flipped)
{
    int rank = Board_Rank(square);
    int file = Board_File(square);
    
    if (flipped)
    {
        rank = 7 - rank;
        file = 7 - file;
    }
    
    return (Vector2){
        BOARD_OFFSET_X + file * SQUARE_SIZE,
        BOARD_OFFSET_Y + rank * SQUARE_SIZE
    };
}

int UI_ScreenToSquare(Vector2 position, bool flipped)
{
    int file = (int)((position.x - BOARD_OFFSET_X) / SQUARE_SIZE);
    int rank = (int)((position.y - BOARD_OFFSET_Y) / SQUARE_SIZE);
    
    if (file < 0 || file >= 8 || rank < 0 || rank >= 8)
        return -1;
    
    if (flipped)
    {
        rank = 7 - rank;
        file = 7 - file;
    }
    
    return Board_Index(rank, file);
}

void UI_DrawBoard(const UIState *ui, bool flipped)
{
    for (int rank = 0; rank < 8; rank++)
    {
        for (int file = 0; file < 8; file++)
        {
            int square = Board_Index(rank, file);
            Vector2 pos = UI_SquareToScreen(square, flipped);
            
            Color color = ((rank + file) % 2 == 0) ? COLOR_LIGHT_SQUARE : COLOR_DARK_SQUARE;
            
            DrawRectangle(pos.x, pos.y, SQUARE_SIZE, SQUARE_SIZE, color);
        }
    }
}

void UI_DrawCoordinates(const UIState *ui, bool flipped)
{
    char files[] = "abcdefgh";
    char ranks[] = "87654321";
    
    if (flipped)
    {
        for (int i = 0; i < 4; i++)
        {
            char temp = files[i];
            files[i] = files[7 - i];
            files[7 - i] = temp;
            
            temp = ranks[i];
            ranks[i] = ranks[7 - i];
            ranks[7 - i] = temp;
        }
    }
    
    /* Draw file letters */
    for (int file = 0; file < 8; file++)
    {
        int x = BOARD_OFFSET_X + file * SQUARE_SIZE + SQUARE_SIZE / 2 - 5;
        int y = BOARD_OFFSET_Y + 8 * SQUARE_SIZE + 5;
        
        char text[2] = {files[file], '\0'};
        DrawText(text, x, y, 20, BLACK);
    }
    
    /* Draw rank numbers */
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
    /* Highlight last move */
    if (ui->lastMove.from >= 0 && ui->lastMove.to >= 0)
    {
        Vector2 fromPos = UI_SquareToScreen(ui->lastMove.from, ui->flipBoard);
        Vector2 toPos = UI_SquareToScreen(ui->lastMove.to, ui->flipBoard);
        
        DrawRectangle(fromPos.x, fromPos.y, SQUARE_SIZE, SQUARE_SIZE, COLOR_LAST_MOVE);
        DrawRectangle(toPos.x, toPos.y, SQUARE_SIZE, SQUARE_SIZE, COLOR_LAST_MOVE);
    }
    
    /* Highlight selected square */
    if (ui->selectedSquare >= 0 && !ui->isDragging)
    {
        Vector2 pos = UI_SquareToScreen(ui->selectedSquare, ui->flipBoard);
        DrawRectangle(pos.x, pos.y, SQUARE_SIZE, SQUARE_SIZE, COLOR_SELECTED);
    }
    
    /* Highlight legal move squares */
    if (ui->showLegalMoves && ui->selectedSquare >= 0)
    {
        for (int i = 0; i < ui->legalMoves.count; i++)
        {
            Move *move = &ui->legalMoves.moves[i];
            if (move->from == ui->selectedSquare)
            {
                Vector2 pos = UI_SquareToScreen(move->to, ui->flipBoard);
                DrawCircle(pos.x + SQUARE_SIZE / 2, pos.y + SQUARE_SIZE / 2, 15, COLOR_LEGAL_MOVE);
            }
        }
    }
    
    /* Highlight king in check */
    if (GameState_IsInCheck(state))
    {
        int kingSquare = GameState_FindKing(state, state->sideToMove);
        if (kingSquare >= 0)
        {
            Vector2 pos = UI_SquareToScreen(kingSquare, ui->flipBoard);
            DrawRectangle(pos.x, pos.y, SQUARE_SIZE, SQUARE_SIZE, COLOR_CHECK);
        }
    }
}

void UI_DrawPiece(const UIState *ui, int pieceValue, int square, Vector2 offset)
{
    int texIndex = UI_GetPieceTextureIndex(pieceValue);
    if (texIndex < 0)
        return;
    
    Vector2 pos = UI_SquareToScreen(square, ui->flipBoard);
    pos.x += offset.x + 5;
    pos.y += offset.y + 5;
    
    DrawTexture(ui->pieces[texIndex], pos.x, pos.y, WHITE);
}

void UI_DrawPieces(const UIState *ui, const GameState *state)
{
    for (int square = 0; square < BOARD_SIZE; square++)
    {
        /* Skip dragged piece */
        if (ui->isDragging && square == ui->draggedSquare)
            continue;
        
        int piece = Board_GetPiece(&state->board, square);
        if (piece != PIECE_NONE)
        {
            UI_DrawPiece(ui, piece, square, (Vector2){0, 0});
        }
    }
    
    /* Draw dragged piece at mouse position */
    if (ui->isDragging && ui->draggedSquare >= 0)
    {
        int piece = Board_GetPiece(&state->board, ui->draggedSquare);
        if (piece != PIECE_NONE)
        {
            Vector2 mousePos = GetMousePosition();
            int texIndex = UI_GetPieceTextureIndex(piece);
            if (texIndex >= 0)
            {
                DrawTexture(ui->pieces[texIndex], 
                           mousePos.x - SQUARE_SIZE / 2, 
                           mousePos.y - SQUARE_SIZE / 2, WHITE);
            }
        }
    }
}

void UI_DrawGameInfo(const UIState *ui, const GameState *state)
{
    int y = BOARD_OFFSET_Y + 8 * SQUARE_SIZE + 40;
    
    /* Side to move */
    const char *sideStr = (state->sideToMove > 0) ? "White to move" : "Black to move";
    DrawText(sideStr, BOARD_OFFSET_X, y, 20, BLACK);
    
    /* Game result */
    if (state->result != GAME_ONGOING)
    {
        const char *resultStr[] = {
            "",
            "White wins by checkmate!",
            "Black wins by checkmate!",
            "Draw by stalemate",
            "Draw by fifty-move rule",
            "Draw by threefold repetition",
            "Draw by insufficient material"
        };
        
        DrawText(resultStr[state->result], BOARD_OFFSET_X, y + 25, 20, RED);
    }
    
    /* Move counter */
    char moveText[64];
    snprintf(moveText, sizeof(moveText), "Move: %d", state->fullmoveNumber);
    DrawText(moveText, BOARD_OFFSET_X + 300, y, 20, BLACK);
    
    /* Check indicator */
    if (GameState_IsInCheck(state) && state->result == GAME_ONGOING)
    {
        DrawText("CHECK!", BOARD_OFFSET_X + 400, y, 20, RED);
    }
}

Move* UI_FindLegalMove(UIState *ui, int from, int to)
{
    for (int i = 0; i < ui->legalMoves.count; i++)
    {
        Move *move = &ui->legalMoves.moves[i];
        if (move->from == from && move->to == to)
        {
            return move;
        }
    }
    return NULL;
}

int UI_PromotionDialog(UIState *ui, int color)
{
    /* Simple promotion selection - click on piece type */
    int pieces[] = {QUEEN, ROOK, BISHOP, KNIGHT};
    const char *names[] = {"Queen", "Rook", "Bishop", "Knight"};
    
    int dialogWidth = 300;
    int dialogHeight = 250;
    int dialogX = (WINDOW_WIDTH - dialogWidth) / 2;
    int dialogY = (WINDOW_HEIGHT - dialogHeight) / 2;
    
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        /* Draw semi-transparent overlay */
        DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, (Color){0, 0, 0, 150});
        
        /* Draw dialog */
        DrawRectangle(dialogX, dialogY, dialogWidth, dialogHeight, LIGHTGRAY);
        DrawRectangleLinesEx((Rectangle){dialogX, dialogY, dialogWidth, dialogHeight}, 3, BLACK);
        
        DrawText("Promote to:", dialogX + 20, dialogY + 20, 20, BLACK);
        
        /* Draw promotion options */
        for (int i = 0; i < 4; i++)
        {
            int buttonY = dialogY + 60 + i * 45;
            Rectangle button = {dialogX + 20, buttonY, dialogWidth - 40, 40};
            
            Color buttonColor = GRAY;
            if (CheckCollisionPointRec(GetMousePosition(), button))
            {
                buttonColor = DARKGRAY;
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                {
                    EndDrawing();
                    return pieces[i];
                }
            }
            
            DrawRectangleRec(button, buttonColor);
            DrawRectangleLinesEx(button, 2, BLACK);
            DrawText(names[i], dialogX + 40, buttonY + 10, 20, WHITE);
        }
        
        EndDrawing();
    }
    
    return QUEEN;  /* Default */
}

void UI_Update(UIState *ui, const GameState *state)
{
    GameState_GenerateLegalMoves((GameState*)state, &ui->legalMoves);
}

bool UI_HandleInput(UIState *ui, GameState *state)
{
    Vector2 mousePos = GetMousePosition();
    int hoveredSquare = UI_ScreenToSquare(mousePos, ui->flipBoard);
    
    /* Start dragging */
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        if (hoveredSquare >= 0)
        {
            int piece = Board_GetPiece(&state->board, hoveredSquare);
            if (piece != PIECE_NONE && Board_PieceColor(piece) == state->sideToMove)
            {
                ui->isDragging = 1;
                ui->draggedSquare = hoveredSquare;
                ui->selectedSquare = hoveredSquare;
                return false;
            }
        }
    }
    
    /* Release drag */
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && ui->isDragging)
    {
        ui->isDragging = 0;
        
        if (hoveredSquare >= 0 && hoveredSquare != ui->draggedSquare)
        {
            Move *move = UI_FindLegalMove(ui, ui->draggedSquare, hoveredSquare);
            if (move)
            {
                /* Handle promotion */
                if (move->flags & MOVE_PROMOTION)
                {
                    move->promotion = UI_PromotionDialog(ui, state->sideToMove);
                }
                
                ui->lastMove = *move;
                GameState_MakeMove(state, move);
                GameState_UpdateResult(state);
                UI_Update(ui, state);
                
                ui->selectedSquare = -1;
                ui->draggedSquare = -1;
                return true;
            }
        }
        
        ui->draggedSquare = -1;
        return false;
    }
    
    /* Click to select/move */
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !ui->isDragging)
    {
        if (hoveredSquare >= 0)
        {
            /* Try to make a move */
            if (ui->selectedSquare >= 0)
            {
                Move *move = UI_FindLegalMove(ui, ui->selectedSquare, hoveredSquare);
                if (move)
                {
                    /* Handle promotion */
                    if (move->flags & MOVE_PROMOTION)
                    {
                        move->promotion = UI_PromotionDialog(ui, state->sideToMove);
                    }
                    
                    ui->lastMove = *move;
                    GameState_MakeMove(state, move);
                    GameState_UpdateResult(state);
                    UI_Update(ui, state);
                    
                    ui->selectedSquare = -1;
                    return true;
                }
            }
            
            /* Select new piece */
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
    
    /* Flip board with F key */
    if (IsKeyPressed(KEY_F))
    {
        ui->flipBoard = !ui->flipBoard;
    }
    
    return false;
}