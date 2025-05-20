#include "raylib.h"
#include "logica_sah.h"
#include "sah_ai.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TILE_SIZE 120
#define WINDOW_SIZE (TILE_SIZE * BOARD_SIZE)
#define MENU_WIDTH (TILE_SIZE * 2)
#define MENU_HEIGHT (TILE_SIZE * BOARD_SIZE)

Texture2D pieceTextures[2][7];
Texture2D avatar;

void LoadPieceTextures() 
{
    const char *names[2][7] = {
        {"AlbPion.png", "AlbCal.png", "AlbNebun.png", "AlbTura.png", "AlbRegina.png", "AlbRege.png"},
        {"NegruPion.png", "NegruCal.png", "NegruNebun.png", "NegruTura.png", "NegruRegina.png", "NegruRege.png"}
    };

    avatar = LoadTexture("assets/NegruPion.png"); 
    if (avatar.id == 0) {
        printf("Failed to load avatar texture: assets/NegruPion.png\n");
        exit(1);
    }
    
    for (int color = 0; color <= 1; color++) 
    {
        for (int type = 1; type <= 6; type++)
        {
            char path[64];
            sprintf(path, "assets/%s", names[color][type - 1]);
            pieceTextures[color][type] = LoadTexture(path);
            if (pieceTextures[color][type].id == 0) 
            {
                printf("Failed to load texture: %s\n", path);
                exit(1);
            }
        }
    }
}

void UnloadPieceTextures()
{
    for (int color = 0; color <= 1; color++)
        for (int type = 1; type <= 6; type++)
            if (pieceTextures[color][type].id != 0)
                UnloadTexture(pieceTextures[color][type]);
    if (avatar.id != 0)
        UnloadTexture(avatar);
}

int main(void) 
{
     InitWindow(600, 400, "Selectare mod de joc");
    SetTargetFPS(60);

    int selectedMode = 0;
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawText("Alege modul de joc:", 120, 40, 30, DARKBLUE);
        DrawText("1. Jucator 1 vs Jucator 2 (aceeasi fereastra)", 60, 100, 20, BLACK);
        DrawText("2. Jucator vs Computer", 60, 140, 20, BLACK);
        DrawText("3. Jucator 1 vs Jucator 2(online)", 60, 180, 20, BLACK);
        DrawText("4. Iesire", 60, 220, 20, BLACK);

        if (IsKeyPressed(KEY_ONE)) selectedMode = 1;
        else if (IsKeyPressed(KEY_TWO)) selectedMode = 2;
        else if (IsKeyPressed(KEY_THREE)) selectedMode = 3;
        else if(IsKeyPressed(KEY_FOUR)) selectedMode = 4;
        if (selectedMode > 0) break;

        EndDrawing();
    }

    CloseWindow();

    if (selectedMode == 1)
    {
    InitWindow(WINDOW_SIZE + MENU_WIDTH, MENU_HEIGHT, "Sah - Grafic");
    SetTargetFPS(60);
    initBoard();
    LoadPieceTextures();

    Vector2 dragging = {-1, -1};
    Vector2 selected = {-1, -1};
    int turn = 0;
    int gameOver = 0;
    int drawRequested = 0;
    int drawResponding = 0;
    char message[64] = "";
    char sahAlert[64] = "";
    char invalidMoveMsg[64] = "";

    while (!WindowShouldClose()) 
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawTextureEx(avatar, (Vector2){20, 20}, 0.0f, 1.2f, WHITE);
        DrawText("Sah - Grafic", 90, 30, 30, BLACK);
        DrawRectangle(WINDOW_SIZE, 0, MENU_WIDTH, MENU_HEIGHT, WHITE);
        DrawText("Restart", WINDOW_SIZE + 10, 20, 20, BLACK);
        DrawText("Salveaza", WINDOW_SIZE + 10, 60, 20, BLACK); 
        DrawText("Iesire", WINDOW_SIZE + 10, 100, 20, BLACK);
        DrawText("Propune remiza", WINDOW_SIZE + 10, 140, 20, BLACK);
        DrawText(TextFormat("Tura %d - %s", turn + 1, (turn % 2 == 0) ? "Alb" : "Negru"), WINDOW_SIZE + 10, 310, 20, BLACK);

        if (drawRequested == 1 && drawResponding == 1) 
        {
            DrawText(TextFormat("Tura %d:Accepta:'D'",turn+2), WINDOW_SIZE + 10, 200, 20, DARKGREEN);
            DrawText(TextFormat("Tura %d:Refuza:'space'",turn+2), WINDOW_SIZE + 10, 240, 20, RED);
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) 
        {
            Vector2 mouse = GetMousePosition();
            if (mouse.x > WINDOW_SIZE) 
            {
                if (mouse.y < 40) 
                {
                    initBoard(); turn = 0; gameOver = 0; message[0] = '\0'; sahAlert[0] = '\0'; invalidMoveMsg[0] = '\0'; drawRequested = 0; drawResponding = 0; historyCount = 0; turnspiecesnottaken = 0;
                } 
                else if (mouse.y < 80) 
                {
                    saveGameToFile("fisier.txt");
                }
                else if (mouse.y < 120) 
                {
                    break;
                } 
                else if (mouse.y < 170 && drawRequested == 0 && !gameOver) 
                {
                    drawRequested = 1;
                    drawResponding = 1;
                }
            } 
            else 
            {
                int col = GetMouseX() / TILE_SIZE;
                int row = GetMouseY() / TILE_SIZE;
                if (row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE) 
                {
                    Piece p = board[row][col];
                    if (selected.x == -1 && selected.y == -1 && p.type == KING && ((turn % 2 == 0 && p.color == P_WHITE) || 
                    (turn % 2 == 1 && p.color == P_BLACK))) 
                    {
                        selected = (Vector2){col, row};
                    } 
                    else if (selected.x != -1 && selected.y != -1 && p.type == ROOK) 
                    {
                        if (castle(col, board[(int)selected.y][(int)selected.x].color)) 
                        {
                            BoardState newState;
                            memcpy(newState.board, board, sizeof(board));
                            history[historyCount++] = newState;
                            turn++;
                            selected = (Vector2){-1, -1};
                            continue;
                        }
                    } 
                    else if (p.type != EMPTY && ((turn % 2 == 0 && p.color == P_WHITE) || (turn % 2 == 1 && p.color == P_BLACK))) 
                    {
                        dragging = (Vector2){col, row};
                        selected = (Vector2){-1, -1};
                    }
                }
            }
        }

        if (drawRequested == 1 && drawResponding == 1) {
            if (IsKeyPressed(KEY_D)) 
            {
                strcpy(message, "Remiza acceptata");
                gameOver = 1;
                drawRequested = 0;
                drawResponding = 0;
            }
            else if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressed(KEY_SPACE)) 
            {
                strcpy(message, "Remiza respinsa");
                drawRequested = 0;
                drawResponding = 0;
            }
        }

        for (int row = 0; row < BOARD_SIZE; row++) 
        {
            for (int col = 0; col < BOARD_SIZE; col++) 
            {
                Color tileColor = ((row + col) % 2 == 0) ? LIGHTGRAY : DARKGRAY;
                DrawRectangle(col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE, tileColor);

                Piece p = board[row][col];
                if (p.type != EMPTY && !(dragging.x == col && dragging.y == row)) {
                    Texture2D texture = pieceTextures[p.color - 1][p.type];
                    float texW = (float)texture.width;
                    float texH = (float)texture.height;
                    float drawScale = (float)TILE_SIZE / texW;
                    float x = col * TILE_SIZE + (TILE_SIZE - texW * drawScale) / 2;
                    float y = row * TILE_SIZE + (TILE_SIZE - texH * drawScale) / 2;
                    DrawTextureEx(texture, (Vector2){x, y}, 0.0f, drawScale, WHITE);
                }
            }
        }

        if (dragging.x != -1 && dragging.y != -1) 
        {
            Piece p = board[(int)dragging.y][(int)dragging.x];
            if (p.type != EMPTY) 
            {
                Texture2D texture = pieceTextures[p.color - 1][p.type];
                float scale = (float)TILE_SIZE / texture.width;
                float drawX = GetMouseX() - (texture.width * scale) / 2;
                float drawY = GetMouseY() - (texture.height * scale) / 2;
                DrawTextureEx(texture, (Vector2){drawX, drawY}, 0.0f, scale, WHITE);
            }
        }

        if (!gameOver && drawRequested == 0) 
        {
            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && dragging.x != -1) 
            {
                int fromCol = (int)dragging.x;
                int fromRow = (int)dragging.y;
                int toCol = GetMouseX() / TILE_SIZE;
                int toRow = GetMouseY() / TILE_SIZE;

                if (toRow >= 0 && toRow < BOARD_SIZE && toCol >= 0 && toCol < BOARD_SIZE &&
                    checkIfMoveIsPossible(fromCol, fromRow, toCol, toRow, turn)) 
                    {
                    Piece moved = board[fromRow][fromCol];
                    Piece captured = board[toRow][toCol];

                    if (moved.type == PAWN && fromCol != toCol && board[toRow][toCol].type == EMPTY) 
                    {
                        if (moved.color == P_WHITE && fromRow == 3 && toRow == 2 && turamutaredubla[toCol + 8] == turn - 1) 
                        {
                            board[toRow + 1][toCol] = (Piece){EMPTY, NONE};
                        } 
                        else if (moved.color == P_BLACK && fromRow == 4 && toRow == 5 && turamutaredubla[toCol] == turn - 1) 
                        {
                            board[toRow - 1][toCol] = (Piece){EMPTY, NONE};
                        }
                    }

                    board[toRow][toCol] = moved;
                    board[fromRow][fromCol] = (Piece){EMPTY, NONE};

                    if (moved.type == KING) 
                    {
                        if (moved.color == P_WHITE) 
                            kingWmoves = 1;
                        else kingBmoves = 1;
                    } 
                    else if (moved.type == ROOK) 
                    {
                        if (moved.color == P_WHITE) 
                        {
                            if (fromRow == 7 && fromCol == 0) 
                                rook1Wmoves = 1;
                            else if (fromRow == 7 && fromCol == 7) 
                                rook2Wmoves = 1;
                        } 
                        else 
                        {
                            if (fromRow == 0 && fromCol == 0) 
                                rook1Bmoves = 1;
                            else if (fromRow == 0 && fromCol == 7) 
                                rook2Bmoves = 1;
                        }
                    }

                    if ((moved.type == PAWN && (toRow == 0 || toRow == BOARD_SIZE - 1))) 
                    {
                        int choosing = 1;
                        PieceType newType = QUEEN;
                        while (choosing && !WindowShouldClose()) 
                        {
                            BeginDrawing();
                            ClearBackground(LIGHTGRAY);
                            DrawText("Alege promotia: Q, R, B, N", 200, 200, 50, DARKBLUE);
                            EndDrawing();
                            if (IsKeyPressed(KEY_Q)) 
                            { 
                            newType = QUEEN; choosing = 0; 
                            }
                            else if (IsKeyPressed(KEY_R)) 
                            { 
                                newType = ROOK; choosing = 0; 
                            }
                            else if (IsKeyPressed(KEY_B)) 
                            { 
                                newType = BISHOP; choosing = 0; 
                            }
                            else if (IsKeyPressed(KEY_N)) 
                            { 
                                newType = KNIGHT; choosing = 0; 
                            }
                        }
                        board[toRow][toCol].type = newType;
                    }

                    if (sah((turn % 2 == 0) ? P_WHITE : P_BLACK)) 
                    {
                        board[fromRow][fromCol] = moved;
                        board[toRow][toCol] = captured;
                        strcpy(invalidMoveMsg, "Miscare invalida:\nnu te poti muta in sah!");
                    } 
                        else 
                    {
                        invalidMoveMsg[0] = '\0';
                        BoardState newState;
                        memcpy(newState.board, board, sizeof(board));
                        history[historyCount++] = newState;

                        if (captured.type != EMPTY)
                            turnspiecesnottaken = 0;
                        else
                            turnspiecesnottaken++;

                        PieceColor opponent = (turn % 2 == 0) ? P_BLACK : P_WHITE;

                        if (sahmat(opponent)) 
                        {
                            sprintf(message, "  Sah mat!\nCastigator: %s", turn % 2 == 0 ? "Alb" : "Negru");
                            gameOver = 1;
                        } 
                        else if (sah(opponent)) 
                        {
                            sprintf(sahAlert, "%s este in sah", turn % 2 == 0 ? "Negru" : "Alb");
                        } 
                        else if (stalemate(opponent, turn+1)) 
                        {
                            sprintf(message, "Remiza automata: pat (stalemate).");
                            gameOver = 1;
                        } 
                        else if (isThreefoldRepetition()) 
                        {
                            sprintf(message, "Remiza automata: pozitie repetata de 3 ori.");
                            gameOver = 1;
                        } 
                        else if (turnspiecesnottaken >= 50) 
                        {
                            sprintf(message, "Remiza automata: 50 de mutari fara capturi.");
                            gameOver = 1;
                        } 
                        else 
                        {
                            sahAlert[0] = '\0';
                        }
                        turn++;
                    }
                }
                dragging = (Vector2){-1, -1};
                selected = (Vector2){-1, -1};
            }
        }
        Vector2 mouse = GetMousePosition();
        if (mouse.x > WINDOW_SIZE) {
            int menuY[] = {20, 60, 100, 140};
            for (int i = 0; i < 4; i++) {
                if (mouse.y > menuY[i] && mouse.y < menuY[i] + 30) {
                    DrawRectangleLines(WINDOW_SIZE + 5, menuY[i] - 5, MENU_WIDTH - 10, 30, RED);
                }
            }
        }
        if (gameOver) 
        {
            DrawText(message, 961, WINDOW_SIZE-200, 28, RED);
            WaitTime(1.1);
        } 
            else 
        {
            if (sahAlert[0] != '\0') 
                DrawText(sahAlert, 961, WINDOW_SIZE - 200, 28, ORANGE);
            if (invalidMoveMsg[0] != '\0') 
                DrawText(invalidMoveMsg, 961, WINDOW_SIZE - 160, 28, RED);
        }
        EndDrawing();
    }

    UnloadTexture(avatar);
    UnloadPieceTextures();
    CloseWindow();
    }
   else if (selectedMode == 2) 
{
    InitWindow(WINDOW_SIZE + MENU_WIDTH, MENU_HEIGHT, "Sah - Jucator vs AI");
    SetTargetFPS(60);
    initBoard();
    LoadPieceTextures();

    Vector2 dragging = {-1, -1};
    Vector2 selected = {-1, -1};
    int playerColor = P_WHITE;
    int aiColor = P_BLACK;
    int turn = 0;
    int gameOver = 0;
    char message[64] = "";
    char sahAlert[64] = "";
    char invalidMoveMsg[64] = "";
    int aiMode = 0;  // 0 = hard, 1 = easy
  
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("  Alege modul:\n[0] Hard, [1] Easy", 350, 400, 40, DARKBLUE);
        EndDrawing();

        if (IsKeyPressed(KEY_ZERO))
        {
            aiMode = 0;
            break;
        }
        if (IsKeyPressed(KEY_ONE)) 
        {
            aiMode = 1;
            break;
        }
    }

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("  Alege culoarea:\n[0] Alb, [1] Negru", 350, 400, 40, DARKBLUE);
        EndDrawing();

        if (IsKeyPressed(KEY_ZERO)) {
            playerColor = P_WHITE;
            aiColor = P_BLACK;
            break;
        }
        if (IsKeyPressed(KEY_ONE)) {
            playerColor = P_BLACK;
            aiColor = P_WHITE;
            break;
        }
    }
    
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        DrawText("Restart", WINDOW_SIZE + 10, 140, 20, BLACK);
        DrawText("Renunta", WINDOW_SIZE + 10, 180, 20, BLACK);
        DrawText("Iesire", WINDOW_SIZE + 10, 220, 20, BLACK);
        
        Vector2 mouse = GetMousePosition();
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (mouse.x > WINDOW_SIZE) {
               if (mouse.y > 120 && mouse.y < 160) 
                 {
                    initBoard();
                    turn = 0;
                    gameOver = 0;
                    message[0] = '\0';
                    sahAlert[0] = '\0';
                    invalidMoveMsg[0] = '\0';
                    EndDrawing();  // ← IMPORTANT!!
                    continue;
                }
                if (mouse.y > 180 && mouse.y < 200) {
                    sprintf(message, "Ai renuntat! Castigator: %s", (turn % 2 == 0) ? "Computer" : "Jucator");
                    gameOver = 1;
                }
                if (mouse.y > 220 && mouse.y < 240) {
                    exit(1);
                }
            }
        }

        for (int row = 0; row < BOARD_SIZE; row++)
            for (int col = 0; col < BOARD_SIZE; col++)
                {
                Color tileColor = ((row + col) % 2 == 0) ? LIGHTGRAY : DARKGRAY;
                DrawRectangle(col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE, tileColor);
                Piece p = board[row][col];
                if (p.type != EMPTY && !(dragging.x == col && dragging.y == row)) {
                    Texture2D texture = pieceTextures[p.color - 1][p.type];
                    float texW = texture.width, texH = texture.height;
                    float scale = (float)TILE_SIZE / texW;
                    DrawTextureEx(texture, (Vector2){col * TILE_SIZE, row * TILE_SIZE}, 0, scale, WHITE);
                }
            }

        if (dragging.x != -1 && dragging.y != -1) 
        {
            Piece p = board[(int)dragging.y][(int)dragging.x];
            Texture2D texture = pieceTextures[p.color - 1][p.type];
            float scale = (float)TILE_SIZE / texture.width;
            DrawTextureEx(texture, (Vector2){GetMouseX() - (texture.width * scale) / 2, GetMouseY() - (texture.height * scale) / 2}, 0, scale, WHITE);
        }

        DrawText(TextFormat("Tura: %s", (turn % 2 == 0) ? "Alb" : "Negru"), WINDOW_SIZE + 10, 20, 20, BLACK);
        if (sahAlert[0]) 
            DrawText(sahAlert, WINDOW_SIZE + 10, 60, 20, ORANGE);
        if (message[0])
            DrawText(message, WINDOW_SIZE + 10, 100, 20, RED);
        if (invalidMoveMsg[0])
            DrawText(invalidMoveMsg, WINDOW_SIZE + 10, 240, 20, RED);

        
        PieceColor current = (turn % 2 == 0) ? P_WHITE : P_BLACK;
        PieceColor opponent = (turn % 2 == 0) ? P_BLACK : P_WHITE;

        if (sahmat(opponent)) 
        {  
            sprintf(message, "Sah mat! Castigator: %s", current == playerColor ? "Jucator" : "Computer");
            gameOver = 1;
        }
        if (isThreefoldRepetition())
        {
        sprintf(message, "Remiza: pozitie repetata de 3 ori.");
        gameOver = 1;
        }
        if (sah(current)) {
            sprintf(sahAlert, "%s este in sah!", current == P_WHITE ? "Alb" : "Negru");
        }
        else if (sah(opponent)) {
            sprintf(sahAlert, "%s este in sah!", opponent == P_WHITE ? "Alb" : "Negru");
        }
        else {
            sahAlert[0] = '\0';
        }
        
         if (mouse.x > WINDOW_SIZE) 
         {
            int menuY[] = {140, 180, 220};
            for (int i = 0; i < 3; i++) {
                if (mouse.y > menuY[i] && mouse.y < menuY[i] + 30)
                {
                    DrawRectangleLines(WINDOW_SIZE + 5, menuY[i] - 5, MENU_WIDTH - 10, 30, RED);
                }
            }
        }
        
        if(!gameOver)
        {
            if ((turn % 2 == 0 && playerColor == P_WHITE) || (turn % 2 == 1 && playerColor == P_BLACK)) 
            {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    int col = GetMouseX() / TILE_SIZE, row = GetMouseY() / TILE_SIZE;
                    if (selected.x == -1 && board[row][col].type == KING && board[row][col].color == playerColor)
                        selected = (Vector2){col, row};
                    else if (selected.x != -1 && board[row][col].type == ROOK && board[row][col].color == playerColor) 
                    {
                        if (castle(col, playerColor)) { turn++; selected = (Vector2){-1, -1}; continue; }
                    }
                    else if (board[row][col].color == playerColor)
                        dragging = (Vector2){col, row};
                }

                if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && dragging.x != -1) 
                {
                    int fromCol = dragging.x, fromRow = dragging.y, toCol = GetMouseX() / TILE_SIZE, toRow = GetMouseY() / TILE_SIZE;
                    if (checkIfMoveIsPossible(fromCol, fromRow, toCol, toRow, turn)) {
                        Piece moved = board[fromRow][fromCol], captured = board[toRow][toCol];
                        if (moved.type == PAWN && fromCol != toCol && board[toRow][toCol].type == EMPTY)
                            board[(moved.color == P_WHITE ? toRow + 1 : toRow - 1)][toCol] = (Piece){EMPTY, NONE};

                        board[toRow][toCol] = moved;
                        board[fromRow][fromCol] = (Piece){EMPTY, NONE};

                        if (moved.type == PAWN && (toRow == 0 || toRow == BOARD_SIZE - 1)) {
                            int choosing = 1; PieceType newType = QUEEN;
                            while (choosing && !WindowShouldClose()) {
                                BeginDrawing(); ClearBackground(LIGHTGRAY);
                                DrawText("Alege promotia: Q, R, B, N", 200, 200, 50, DARKBLUE); EndDrawing();
                                if (IsKeyPressed(KEY_Q)) { newType = QUEEN; choosing = 0; }
                                else if (IsKeyPressed(KEY_R)) { newType = ROOK; choosing = 0; }
                                else if (IsKeyPressed(KEY_B)) { newType = BISHOP; choosing = 0; }
                                else if (IsKeyPressed(KEY_N)) { newType = KNIGHT; choosing = 0; }
                            }
                            board[toRow][toCol].type = newType;
                        }

                        if (sah(playerColor)) {
                            board[fromRow][fromCol] = moved;
                            board[toRow][toCol] = captured;
                            strcpy(invalidMoveMsg, "Miscare invalida: sah!");
                        } else {
                            invalidMoveMsg[0] = '\0';
                            turn++;
                        }
                    }
                    dragging = (Vector2){-1, -1};
                    selected = (Vector2){-1, -1};
                }
            }
        
             else 
            {
                if (!gameOver) 
                {
                    //WaitTime(0.1); //gandire AI
                    if (aiMode == 0)
                        handleAIMove(aiColor);
                    else
                        handleRandomAIMove(aiColor);
                    turn++;
                }
            }
             
        }
        
        else
        {
            Vector2 mouse = GetMousePosition();
            DrawText(message, 961, WINDOW_SIZE - 200, 25, RED);
            DrawText("Click pe\nRestart/Renunta/Iesire", 961, WINDOW_SIZE - 160, 25, BLACK);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) 
            {
                if (mouse.x > WINDOW_SIZE) {
                    if (mouse.y > 140 && mouse.y < 180) {
                        initBoard();
                        turn = 0;
                        gameOver = 0;
                        message[0] = '\0';
                        sahAlert[0] = '\0';
                        invalidMoveMsg[0] = '\0';
                    }
                    if (mouse.y > 180 && mouse.y < 220) {
                        break;
                    }
                }
            }
            continue;
        }
        EndDrawing();
    }
  
    UnloadPieceTextures();
    CloseWindow();
}
    else if (selectedMode == 3)
    {
        /*
        InitWindow(WINDOW_SIZE + MENU_WIDTH, MENU_HEIGHT, "Sah Online");
        SetTargetFPS(60);
        initBoard();
        LoadPieceTextures();

       int socket = init_client("127.0.0.1", 12345);
        if (socket < 0)
        {
            DrawText("Conectare esuata.", 50, 100, 20, RED);
            WaitTime(2.0);
            CloseWindow();
            return 1;
        }

        int myColor = 0;
        recv(socket, &myColor, sizeof(int), 0);


        int playerTurn = 0; // 0 - Alb, 1 - Negru
        Vector2 dragging = {-1, -1};

        while (!WindowShouldClose())
        {
            BeginDrawing();
            ClearBackground(RAYWHITE);

            for (int row = 0; row < BOARD_SIZE; row++)
                for (int col = 0; col < BOARD_SIZE; col++)
                    DrawRectangle(col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE, (row + col) % 2 ? GRAY : LIGHTGRAY);

            for (int row = 0; row < BOARD_SIZE; row++)
            {
                for (int col = 0; col < BOARD_SIZE; col++)
                {
                    Piece p = board[row][col];
                    if (p.type != EMPTY)
                    {
                        Texture2D tex = pieceTextures[p.color - 1][p.type];
                        float scale = (float)TILE_SIZE / tex.width;
                        DrawTextureEx(tex, (Vector2){col * TILE_SIZE, row * TILE_SIZE}, 0.0f, scale, WHITE);
                    }
                }
            }

            EndDrawing();

            if ((playerTurn % 2) == myColor)
            {
                if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
                {
                    int fromCol = GetMouseX() / TILE_SIZE;
                    int fromRow = GetMouseY() / TILE_SIZE;

                    if (board[fromRow][fromCol].color == myColor + 1)
                    {
                        dragging = (Vector2){fromCol, fromRow};
                    }
                    else if (dragging.x != -1 && dragging.y != -1)
                    {
                        int toCol = fromCol;
                        int toRow = fromRow;

                        if (checkIfMoveIsPossible((int)dragging.x, (int)dragging.y, toCol, toRow, playerTurn))
                        {
                            Move m = {(int)dragging.y, (int)dragging.x, toRow, toCol};
                            send_move(socket, &m);

                            board[toRow][toCol] = board[(int)dragging.y][(int)dragging.x];
                            board[(int)dragging.y][(int)dragging.x] = (Piece){EMPTY, NONE};

                            playerTurn++;
                            dragging = (Vector2){-1, -1};
                        }
                    }
                }
            }
            else
            {
                Move m;
                if (receive_move(socket, &m) > 0)
                {
                    board[m.toRow][m.toCol] = board[m.fromRow][m.fromCol];
                    board[m.fromRow][m.fromCol] = (Piece){EMPTY, NONE};
                    playerTurn++;
                }
            }
        }

        UnloadPieceTextures();
        CloseWindow();
        close_socket(socket);
        */
        InitWindow(600, 200, "Modul Multiplayer");
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Inca nu este implementat Jucator 1 vs Jucator 2.", 30, 80, 20, BLACK);
        EndDrawing();
        WaitTime(2.0);
        CloseWindow();
        main();
    }
    else
    {
        InitWindow(600, 200, "Iesire");
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Vei fi scos numaidecat. La revedere!", 30, 80, 20, RED);
        EndDrawing();
        WaitTime(2.0);
        CloseWindow();
    }
    return 0;
}