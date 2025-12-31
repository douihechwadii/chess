#include <stdio.h>
#include "raylib.h"
#include "board.h"
#include "movegen.h"
#include "gamestate.h"
#include "ui.h"
#include "ai.h"

typedef enum {
    SCREEN_MENU,
    SCREEN_GAME,
    SCREEN_GAME_OVER
} GameScreen;

typedef enum {
    MODE_HUMAN_VS_HUMAN,
    MODE_HUMAN_VS_AI
} GameMode;

typedef struct {
    Rectangle rect;
    const char *text;
    Color color;
    Color hoverColor;
} Button;

bool IsButtonPressed(Button *btn, Vector2 mousePos, bool mousePressed)
{
    bool hover = CheckCollisionPointRec(mousePos, btn->rect);
    
    if (hover && mousePressed) {
        return true;
    }
    
    return false;
}

void DrawButton(Button *btn, Vector2 mousePos)
{
    bool hover = CheckCollisionPointRec(mousePos, btn->rect);
    Color color = hover ? btn->hoverColor : btn->color;
    
    DrawRectangleRec(btn->rect, color);
    DrawRectangleLinesEx(btn->rect, 2, BLACK);
    
    int textWidth = MeasureText(btn->text, 20);
    DrawText(btn->text, 
             btn->rect.x + (btn->rect.width - textWidth) / 2,
             btn->rect.y + (btn->rect.height - 20) / 2,
             20, BLACK);
}

int main(void)
{
    // Initialize window 
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Chess Game");
    SetTargetFPS(60);
    
    GameScreen currentScreen = SCREEN_MENU;
    GameMode gameMode = MODE_HUMAN_VS_AI;
    int humanColor = 1;  // 1 = white, -1 = black
    
    // Initialize game state 
    GameState state;
    UIState ui;
    AIState ai;
    
    bool aiThinking = false;
    Move aiMove;
    double aiThinkStartTime = 0;
    
    // Menu buttons
    float btnWidth = 300;
    float btnHeight = 50;
    float centerX = (WINDOW_WIDTH - btnWidth) / 2;
    
    Button btnHumanVsHuman = {
        {centerX, 200, btnWidth, btnHeight},
        "Human vs Human", LIGHTGRAY, GRAY
    };
    
    Button btnHumanVsAI = {
        {centerX, 270, btnWidth, btnHeight},
        "Human vs AI", SKYBLUE, BLUE
    };
    
    float colorBtnWidth = 150;
    float colorBtnHeight = 40;
    float colorBtnSpacing = 20;
    float colorBtnStartX = (WINDOW_WIDTH - (colorBtnWidth * 2 + colorBtnSpacing)) / 2;
    
    Button btnPlayAsWhite = {
        {colorBtnStartX, 360, colorBtnWidth, colorBtnHeight},
        "Play as White", SKYBLUE, BLUE
    };
    
    Button btnPlayAsBlack = {
        {colorBtnStartX + colorBtnWidth + colorBtnSpacing, 360, colorBtnWidth, colorBtnHeight},
        "Play as Black", LIGHTGRAY, GRAY
    };
    
    Button btnStart = {
        {centerX, 430, btnWidth, btnHeight},
        "START GAME", GREEN, DARKGREEN
    };
    
    Button btnQuit = {
        {centerX, 500, btnWidth, btnHeight},
        "QUIT", RED, MAROON
    };
    
    Button btnNewGame = {
        {centerX, 570, btnWidth, btnHeight},
        "NEW GAME", GREEN, DARKGREEN
    };
    
    printf("Chess Game Started!\n");
    
    // Main game loop 
    while (!WindowShouldClose())
    {
        Vector2 mousePos = GetMousePosition();
        bool mousePressed = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
        
        // ==== MENU SCREEN ====
        if (currentScreen == SCREEN_MENU)
        {
            // Handle button clicks
            if (IsButtonPressed(&btnHumanVsHuman, mousePos, mousePressed)) {
                gameMode = MODE_HUMAN_VS_HUMAN;
                btnHumanVsHuman.color = SKYBLUE;
                btnHumanVsAI.color = LIGHTGRAY;
            }
            
            if (IsButtonPressed(&btnHumanVsAI, mousePos, mousePressed)) {
                gameMode = MODE_HUMAN_VS_AI;
                btnHumanVsHuman.color = LIGHTGRAY;
                btnHumanVsAI.color = SKYBLUE;
            }
            
            if (gameMode == MODE_HUMAN_VS_AI) {
                if (IsButtonPressed(&btnPlayAsWhite, mousePos, mousePressed)) {
                    humanColor = 1;
                    btnPlayAsWhite.color = SKYBLUE;
                    btnPlayAsBlack.color = LIGHTGRAY;
                }
                
                if (IsButtonPressed(&btnPlayAsBlack, mousePos, mousePressed)) {
                    humanColor = -1;
                    btnPlayAsWhite.color = LIGHTGRAY;
                    btnPlayAsBlack.color = SKYBLUE;
                }
            }
            
            if (IsButtonPressed(&btnStart, mousePos, mousePressed)) {
                // Initialize game
                GameState_Init(&state);
                UI_Init(&ui);
                UI_Update(&ui, &state);
                AI_Init(&ai, AI_MEDIUM);  // Always use medium difficulty
                
                aiThinking = false;
                
                currentScreen = SCREEN_GAME;
                
                printf("\n=== NEW GAME ===\n");
                if (gameMode == MODE_HUMAN_VS_HUMAN) {
                    printf("Mode: Human vs Human\n");
                } else {
                    printf("Mode: Human vs AI (Medium)\n");
                    printf("You are: %s\n", humanColor > 0 ? "White" : "Black");
                }
                printf("================\n\n");
            }
            
            if (IsButtonPressed(&btnQuit, mousePos, mousePressed)) {
                break;
            }
            
            // Draw menu
            BeginDrawing();
            ClearBackground(RAYWHITE);
            
            DrawText("CHESS GAME", centerX + 40, 100, 40, BLACK);
            
            DrawText("Select Game Mode:", centerX + 60, 170, 20, DARKGRAY);
            DrawButton(&btnHumanVsHuman, mousePos);
            DrawButton(&btnHumanVsAI, mousePos);
            
            if (gameMode == MODE_HUMAN_VS_AI) {
                DrawText("Play as:", (WINDOW_WIDTH - MeasureText("Play as:", 20)) / 2, 335, 20, DARKGRAY);
                DrawButton(&btnPlayAsWhite, mousePos);
                DrawButton(&btnPlayAsBlack, mousePos);
            }
            
            DrawButton(&btnStart, mousePos);
            DrawButton(&btnQuit, mousePos);
            
            EndDrawing();
        }
        
        // ==== GAME SCREEN ====
        else if (currentScreen == SCREEN_GAME)
        {
            // Handle AI move
            if (gameMode == MODE_HUMAN_VS_AI && 
                state.sideToMove != humanColor &&
                state.result == GAME_ONGOING &&
                !aiThinking)
            {
                aiThinking = true;
                aiThinkStartTime = GetTime();
                aiMove = AI_GetBestMove(&ai, &state);
            }
            
            // Execute AI move
            if (aiThinking && GetTime() - aiThinkStartTime > 0.3)
            {
                if (aiMove.from >= 0) {
                    ui.lastMove = aiMove;
                    GameState_MakeMove(&state, &aiMove);
                    GameState_UpdateResult(&state);
                    UI_Update(&ui, &state);
                    
                    char files[] = "abcdefgh";
                    printf("AI moved: %c%d -> %c%d\n",
                           files[Board_File(aiMove.from)],
                           8 - Board_Rank(aiMove.from),
                           files[Board_File(aiMove.to)],
                           8 - Board_Rank(aiMove.to));
                    
                    if (state.result != GAME_ONGOING) {
                        currentScreen = SCREEN_GAME_OVER;
                    }
                }
                aiThinking = false;
            }
            
            // Handle human input
            if (gameMode == MODE_HUMAN_VS_HUMAN ||
                (gameMode == MODE_HUMAN_VS_AI && state.sideToMove == humanColor))
            {
                bool moveMade = UI_HandleInput(&ui, &state);
                
                if (moveMade)
                {
                    char files[] = "abcdefgh";
                    printf("Move: %c%d -> %c%d\n",
                           files[Board_File(ui.lastMove.from)],
                           8 - Board_Rank(ui.lastMove.from),
                           files[Board_File(ui.lastMove.to)],
                           8 - Board_Rank(ui.lastMove.to));
                    
                    if (state.result != GAME_ONGOING) {
                        currentScreen = SCREEN_GAME_OVER;
                    }
                }
            }
            
            // Draw game
            BeginDrawing();
            ClearBackground(RAYWHITE);
            
            UI_DrawBoard(&ui);
            UI_DrawHighlights(&ui, &state);
            UI_DrawPieces(&ui, &state);
            UI_DrawCoordinates(&ui);
            UI_DrawGameInfo(&ui, &state);
            
            if (aiThinking) {
                DrawText("AI is thinking...", 
                         BOARD_OFFSET_X + 250, 
                         BOARD_OFFSET_Y + (SQUARE_SIZE * 8) + 50, 
                         20, DARKGRAY);
            }
            
            EndDrawing();
        }
        
        // ==== GAME OVER SCREEN ====
        else if (currentScreen == SCREEN_GAME_OVER)
        {
            if (IsButtonPressed(&btnNewGame, mousePos, mousePressed)) {
                UI_Cleanup(&ui);
                currentScreen = SCREEN_MENU;
            }
            
            if (IsButtonPressed(&btnQuit, mousePos, mousePressed)) {
                break;
            }
            
            BeginDrawing();
            ClearBackground(RAYWHITE);
            
            UI_DrawBoard(&ui);
            UI_DrawHighlights(&ui, &state);
            UI_DrawPieces(&ui, &state);
            UI_DrawCoordinates(&ui);
            UI_DrawGameInfo(&ui, &state);
            
            // Draw semi-transparent overlay
            DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, (Color){0, 0, 0, 150});
            
            // Game over message
            const char *resultStr[] = {
                "",
                "WHITE WINS!",
                "BLACK WINS!",
                "STALEMATE - DRAW",
                "DRAW - 50 MOVE RULE",
                "DRAW - REPETITION",
                "DRAW - INSUFFICIENT MATERIAL"
            };
            
            const char *msg = resultStr[state.result];
            int msgWidth = MeasureText(msg, 40);
            DrawText(msg, (WINDOW_WIDTH - msgWidth) / 2, 200, 40, WHITE);
            
            DrawButton(&btnNewGame, mousePos);
            DrawButton(&btnQuit, mousePos);
            
            EndDrawing();
        }
    }
    
    // Cleanup 
    if (currentScreen != SCREEN_MENU) {
        UI_Cleanup(&ui);
    }
    CloseWindow();
    
    printf("Chess Game Closed.\n");
    
    return 0;
}