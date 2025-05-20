#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include "logica_sah.h"

void play() //functia de joc
{
    int turn=0;
    while(1) 
    {
        printBoard();
        PieceColor currentPlayer = (turn % 2 == 0) ? WHITE : BLACK;
        PieceColor opponentPlayer = (turn % 2 == 0) ? BLACK : WHITE;

    if (sah(currentPlayer, turn)) 
    {
        printf("Jucatorul %s se afla in sah\n", currentPlayer == WHITE ? "Alb" : "Negru");
        if (sahmat(currentPlayer, turn)) 
        {
          break;
        }
    
        inputstage:
        printf("Jucatorul %s, alegeti o miscare (exemplu: 'A2 A4' sau 'exit game' pentru iesire):\n", currentPlayer == WHITE ? "Alb" : "Negru");
    
        char move[20];
        if(fgets(move, sizeof(move), stdin)==NULL)
        {
            perror("Eroare citire miscare\n");
        }
    
        char initialColumnChar, finalColumnChar;
        int initialRow, finalRow;
        int initialColumn, finalColumn;
        
        if (strncmp(move,"exit game",9) == 0)//iesirea din joc
        {
            printf("Meciul a fost abandonat.\n");    
            exit(1);
        }
        
        if (sscanf(move, "%c%d %c%d", &initialColumnChar, &initialRow, &finalColumnChar, &finalRow) != 4) 
        {
            printf("Format invalid! Folositi formatul 'A2 A4'.\n");
            goto inputstage;
        }
    
        initialColumn = toupper(initialColumnChar) - 'A';
        finalColumn = toupper(finalColumnChar) - 'A';
        initialRow = BOARD_SIZE - initialRow;
        finalRow = BOARD_SIZE - finalRow;
    
        if (!isInsideBoard(initialRow, initialColumn) || !isInsideBoard(finalRow, finalColumn)) 
        {
            printf("Mutare in afara tablei!\n");
            goto inputstage;
        }
    
        if (board[initialRow][initialColumn].color != currentPlayer) 
        {
            printf("Nu puteti muta piesele adversarului!\n");
            goto inputstage;
        }
    
        Piece initialPiece = board[initialRow][initialColumn];
        Piece finalPiece = board[finalRow][finalColumn];
    
        board[finalRow][finalColumn] = initialPiece;
        board[initialRow][initialColumn] = (Piece){EMPTY, NONE};
    
        if (sah(currentPlayer, turn)) 
        {
            printf("Miscare invalida! Sunteti inca in sah. Incercati din nou.\n");
            board[initialRow][initialColumn] = initialPiece; 
            board[finalRow][finalColumn] = finalPiece;
            goto inputstage;
        }
        turn++;
        continue;
    }
    
        input:
        printf("Jucatorul %s, alegeti o miscare (exemplu: 'A2 A4', 'castle left', 'draw' pentru remiza, 'save game' pentru salvare in fisier text sau 'exit game' pentru iesire):\n", currentPlayer == WHITE ? "Alb" : "Negru");
        char move[20];
        if(fgets(move, sizeof(move), stdin)==NULL)
        {
            perror("Eroare citire miscare\n");
        }
        
        if (strncmp(move,"castle left",11) == 0) 
        {
            if (castle(0, currentPlayer)==1)
            {
                printf("Rocada la stanga reusita!\n");
                turn++;
                continue;
            }
            else 
            {
                printf("Rocada nu este posibila!\n");
                continue;
            }
        }
        else if (strncmp(move, "castle right", 12) == 0) 
        {
            if (castle(7, currentPlayer)==1)
            {
                printf("Rocada la dreapta reusita!\n");
                turn++;
                continue;
            }
            else 
            {
                printf("Rocada nu este posibila!\n");
                continue;
            }
        }
        else if (strncmp(move, "save game", 9) == 0)
        {
            saveGameToFile("chess_game.txt");
            printf("salvat in fisierul chess_game.txt\n");
            continue;
        }
        else  if (strncmp(move,"exit game",9) == 0)//iesirea din joc
        {
            printf("Meciul a fost abandonat.\n");    
            exit(1);
        }
        
        else if (strncmp(move, "draw", 4) == 0)//sistemul de remiza prin votare
        {
            printf("Jucatorul %s, doriti remiza votata de jucatorul %s? (Scrieti 'draw' pentru acceptare, orice altceva pentru refuz)\n", 
                   currentPlayer == WHITE ? "Negru" : "Alb",
                   currentPlayer == WHITE ? "Alb" : "Negru");
        
            char move1[20];
            if(fgets(move1, sizeof(move1), stdin)==NULL) 
            {
            perror("Eroare citire miscare\n");
            }
            move1[strcspn(move1, "\n")] = 0;  
        
            if (strcmp(move1,"draw") == 0)
            {
                printf("Jocul s-a incheiat in remiza(decizie comuna a celor 2 jucatori).\n");
                exit(0);
            }
            else
            {
                printf("Remiza nu a fost acceptata.\n");
                continue;
            }
        }
        
        char initialColumnChar, finalColumnChar;
        int initialRow, finalRow;
        int initialColumn, finalColumn;
        if (sscanf(move, "%c%d %c%d", &initialColumnChar, &initialRow, &finalColumnChar, &finalRow) != 4) 
        {
            printf("Input invalid. Incercati din nou.\n");
            continue;
        }
        
        initialColumn = toupper(initialColumnChar) - 'A' + 1;
        finalColumn = toupper(finalColumnChar) - 'A' + 1;
        initialRow = BOARD_SIZE - initialRow;
        initialColumn=initialColumn-1;  
        finalRow = BOARD_SIZE - finalRow;
        finalColumn=finalColumn-1; 
        
        if (board[initialRow][initialColumn].type == PAWN && abs(finalColumn - initialColumn) == 1) 
        {
            if (board[initialRow][finalColumn].type == PAWN && board[initialRow][finalColumn].color != currentPlayer) 
            {
                if (currentPlayer == WHITE && initialRow == 3 && finalRow == 2 && turamutaredubla[8 + finalColumn] == turn - 1) 
                {
                    board[initialRow][finalColumn] = (Piece){EMPTY, NONE}; 
                    printf("Captura en-passant!\n");
                }
                else if (currentPlayer == BLACK && initialRow == 4 && finalRow == 5 && turamutaredubla[finalColumn] == turn - 1) 
                {
                    board[initialRow][finalColumn] = (Piece){EMPTY, NONE}; 
                    printf("Captura en-passant!\n");
                }
            }
        }

        if (checkIfMoveIsPossible(initialColumn, initialRow, finalColumn, finalRow, turn)) 
        {
            if (board[initialRow][initialColumn].type == KING)
            {
                if(currentPlayer == WHITE)
                {
                    int dr = abs(finalRow - rowKingB);
                    int dc = abs(finalColumn - colKingB);
                    if (dr <= 1 && dc <= 1)
                    {
                        printf("Miscare invalida, regii sunt prea apropiati.\n");
                        goto input;
                    }
                    if(isSquareAttacked(finalRow,finalColumn,BLACK))
                    {
                        printf("Miscare invalida, nu se poate muta in sah.\n");
                        goto input;
                    }
                    colKingW=finalColumn;
                    rowKingW=finalRow;
                }
                else
                {
                    int dr = abs(finalRow - rowKingW);
                    int dc = abs(finalColumn - colKingW);
                    if (dr <= 1 && dc <= 1)
                    {
                        printf("Miscare invalida, regii sunt prea apropiati.\n");
                        goto input;
                    }
                    if(isSquareAttacked(finalRow,finalColumn,WHITE))
                    {
                        printf("Miscare invalida, nu se poate muta in sah.\n");
                        goto input;
                    }
                    colKingB=finalColumn;
                    rowKingB=finalRow;
                }
            }
            
            if(initialColumn==0 && initialRow==0)
                rook1Wmoves=1;
            
            if(initialColumn==7 && initialRow==0)
                rook2Wmoves=1;
        
            if(initialColumn==0 && initialRow==7)
                rook1Bmoves=1;
            
            if(initialColumn==7 && initialRow==7)
                rook2Bmoves=1;
            
            if(initialColumn==7 && initialRow==4)
               kingBmoves=1;
               
            if(initialColumn==0 && initialRow==4)
                kingWmoves=1;
                
            if (board[finalRow][finalColumn].type!=EMPTY && board[finalRow][finalColumn].color==opponentPlayer) 
            {
                turnspiecesnottaken=0; //resetam nr de ture de cand nu s-a mai luat o piesa
            }
                
            board[finalRow][finalColumn]=board[initialRow][initialColumn]; //mutarea efectiva
            board[initialRow][initialColumn]=(Piece){EMPTY, NONE}; 
        } 
        else
        {
            printf("Miscare invalidă. Incercați din nou.\n");
            continue;
        }
    
        if (board[finalRow][finalColumn].type == PAWN && (finalRow == 0 || finalRow == BOARD_SIZE-1))//promovarea pionilor
        {
            char choice;
            PieceColor currentColor = (turn % 2 == 0) ? WHITE : BLACK;
        
            printf("Alegeti promovarea pentru %s (Q pentru regina, R pentru tura, B pentru nebun, N pentru cal): ",
                   currentColor == WHITE ? "Alb" : "Negru");
            if(scanf("%c", &choice)!=1)
            {
            perror("Eroare citire piesa\n");
            }
        
            if (currentColor==WHITE) 
            {
                choice=toupper(choice);
                switch(choice) 
                {
                    case 'Q': 
                        board[finalRow][finalColumn].type = QUEEN;
                        break;
                    case 'R': 
                        board[finalRow][finalColumn].type = ROOK;  
                        break;
                    case 'B': 
                        board[finalRow][finalColumn].type = BISHOP;
                        break;
                    case 'N': 
                        board[finalRow][finalColumn].type = KNIGHT;
                        break;
                    default:  
                        board[finalRow][finalColumn].type = QUEEN;
                        break; 
                }
            } 
            else 
            {
                choice=tolower(choice);
                switch (choice) 
                {
                    case 'q': 
                        board[finalRow][finalColumn].type = QUEEN; 
                        break;
                    case 'r': 
                        board[finalRow][finalColumn].type = ROOK;
                        break;
                    case 'b': 
                        board[finalRow][finalColumn].type = BISHOP; 
                        break;
                    case 'n':
                        board[finalRow][finalColumn].type = KNIGHT; 
                        break;
                    default:  
                        board[finalRow][finalColumn].type = QUEEN;
                        break; 
                }
            }
        board[finalRow][finalColumn].color = currentColor; 
        }
    
        if(stalemate(currentPlayer,turn))
        {
            printf("Jocul s-a incheiat in remiza(Pat).\n");
        }
        checkThreefoldRepetition();
    
        turnspiecesnottaken++;
        if(turnspiecesnottaken>=50)
        {
            printf("Jocul s-a incheiat in remiza(nu au fost capturate piese timp de 50 de ture).\n");
            exit(1);
        }
        
        turn++;
    }
}

int main(void)
{
    inceputjoc:
    ;
    int n;
    printf("Introduceti modul de joc pe care vreti sa il jucati:\n1. Jucator 1 vs Jucator 2(aceeasi consola)\n2. Jucator 1 vs Jucator 2(online)\n3. Jucator vs Computer\n");
    if(scanf("%d",&n)!=1)
    {
        perror("eroare introducere mod de joc");
    }
    getchar();//elimina '\n' din bufferul de intrare
    
    if(n==1)
    {
        printf("\n");
        initBoard();
        play();
        printf("\n");
    }
    
    else if(n==2)
    {
        printf("Inca nu este implementat modul Online.\n");
    }
    
    else if(n==3)
    {
        printf("Inca nu este implementat Jucator vs Computer.\n");
    }
    
    else
    {
        printf("Nu ai introdus corect modul de joc, mai incearca.\n");
        goto inceputjoc;
    }
    
    return 0;
}