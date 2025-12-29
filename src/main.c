#include <stdio.h>
#include "raylib.h"
#include "board.h"
#include "movegen.h"
#include "gamestate.h"
#include "ui.h"

int main(void)
{
    // Initialize window 
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Chess Game");
    SetTargetFPS(60);
    
    // Initialize game state 
    GameState state;
    GameState_Init(&state);
    
    // Initialize UI 
    UIState ui;
    UI_Init(&ui);
    UI_Update(&ui, &state);
    
    printf("Chess Game Started!\n");
    printf("Controls:\n");
    printf("  - Click a piece, then click destination\n");
    printf("  - Close window to exit\n\n");
    
    // Main game loop 
    while (!WindowShouldClose())
    {
        // Handle input 
        bool moveMade = UI_HandleInput(&ui, &state);
        
        if (moveMade)
        {
            printf("Move made: ");
            char files[] = "abcdefgh";
            printf("%c%d -> %c%d\n",
                   files[Board_File(ui.lastMove.from)],
                   8 - Board_Rank(ui.lastMove.from),
                   files[Board_File(ui.lastMove.to)],
                   8 - Board_Rank(ui.lastMove.to));
            
            if (state.result != GAME_ONGOING)
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
                printf("\nGame Over: %s\n\n", resultStr[state.result]);
            }
        }
        
        // Draw everything 
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        UI_DrawBoard(&ui);
        UI_DrawHighlights(&ui, &state);
        UI_DrawPieces(&ui, &state);
        UI_DrawCoordinates(&ui);
        UI_DrawGameInfo(&ui, &state);
        
        EndDrawing();
    }
    
    // Cleanup 
    UI_Cleanup(&ui);
    CloseWindow();
    
    printf("Chess Game Closed.\n");
    
    return 0;
}