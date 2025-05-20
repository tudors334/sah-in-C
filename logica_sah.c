#include "logica_sah.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define BOARD_SIZE 8
#define MAX_MOVES 1000 //media nr de mutari e 40 intr-un meci de sah

BoardState history[MAX_MOVES];//istoric pozitii(pt tripla repetare)
int historyCount=0;

Piece board[BOARD_SIZE][BOARD_SIZE];

void initBoard() //initializarea tablei cu toate piesele la locurile standard
{
    for (int i = 0; i < BOARD_SIZE; i++)
        for (int j = 0; j < BOARD_SIZE; j++)
            board[i][j] = (Piece){EMPTY, NONE};
            
    //initializare negru
    board[0][0] = board[0][7] = (Piece){ROOK, P_BLACK};
    board[0][1] = board[0][6] = (Piece){KNIGHT, P_BLACK};
    board[0][2] = board[0][5] = (Piece){BISHOP, P_BLACK};
    board[0][3] = (Piece){QUEEN, P_BLACK};
    board[0][4] = (Piece){KING, P_BLACK};
    for (int j = 0; j < BOARD_SIZE; j++)
        board[1][j] = (Piece){PAWN, P_BLACK};

    //initializare alb
    board[7][0] = board[7][7] = (Piece){ROOK, P_WHITE};
    board[7][1] = board[7][6] = (Piece){KNIGHT, P_WHITE};
    board[7][2] = board[7][5] = (Piece){BISHOP, P_WHITE};
    board[7][3] = (Piece){QUEEN, P_WHITE};
    board[7][4] = (Piece){KING, P_WHITE};
    for (int j = 0; j < BOARD_SIZE; j++)
        board[6][j] = (Piece){PAWN, P_WHITE};
}

void printBoard() //afisarea tablei pentru fiecare pas
{
    const char *symbol;
    printf("    A B C D E F G H\n");
    printf("    - - - - - - - -\n");
    for (int i = 0; i < BOARD_SIZE; i++) 
    {
        printf("%d | ", 8 - i);
        for (int j = 0; j < BOARD_SIZE; j++) 
        {
            Piece p = board[i][j];
            if (p.type == EMPTY) 
            {
            if ((i + j) % 2 == 0)
                symbol = "□"; //alb
            else
                symbol = "■"; //negru
            } 
            else 
            {
                switch (p.type) 
                {
                    case PAWN:   
                        symbol = (p.color == P_WHITE)?"♙":"♟";
                        break;//pion
                    case KNIGHT:
                        symbol = (p.color == P_WHITE)?"♘":"♞";
                        break; //cal
                    case BISHOP: 
                        symbol = (p.color == P_WHITE)?"♖":"♝"; 
                        break; //nebun
                    case ROOK:   
                        symbol = (p.color == P_WHITE)?"♖":"♜";
                        break; //tura
                    case QUEEN:  
                        symbol = (p.color == P_WHITE)?"♕":"♛";
                        break; //regina
                    case KING:   
                        symbol = (p.color == P_WHITE)?"♔":"♚"; 
                        break; //rege
                    default: 
                        symbol = "?"; 
                        break; 
                }
            }
            printf("%s ", symbol);
        }
        printf("| %d\n", 8 - i);
    }
    printf("    - - - - - - - -\n");
    printf("    A B C D E F G H\n");
}

int isPathClear(int initialColumn, int initialRow, int finalColumn, int finalRow)//verifica dace poti muta fara sa treci prin piese
{
    int rowStep = (finalRow > initialRow) ? 1 : (finalRow < initialRow) ? -1 : 0;
    int colStep = (finalColumn > initialColumn) ? 1 : (finalColumn < initialColumn) ? -1 : 0;
    
    int r=initialRow+rowStep;
    int c=initialColumn+colStep;
    
    while (r!=finalRow || c!=finalColumn) 
    {
        if (board[r][c].type!=EMPTY)
            return 0;
        r=r+rowStep;
        c=c+colStep;
    }
    return 1;
}

int turamutaredubla[16]={0};//pentru en passant(primele 8 pt alb si ultimele 8 pt negru)

int checkIfMoveIsPossible(int initialColumn, int initialRow, int finalColumn, int finalRow, int turn) //functie care verifica daca e posibila o mutare de la A la B
{
    PieceColor currentPlayer=(turn % 2 == 0) ? P_WHITE : P_BLACK;
    
    Piece piece=board[initialRow][initialColumn];
    if (piece.type==EMPTY || piece.color!=currentPlayer)
        return 0;
    
    Piece target = board[finalRow][finalColumn];
    if (target.color==currentPlayer)
        return 0;
            
    int rowDiff = abs(finalRow - initialRow);
    int colDiff = abs(finalColumn - initialColumn);
    
    switch (piece.type)
    {
       case PAWN:
            if (piece.color == P_BLACK) 
            {
                if (initialRow == 1 && finalRow == 3 && finalColumn == initialColumn && 
                    target.type == EMPTY && board[2][initialColumn].type == EMPTY) 
                {
                    turamutaredubla[8 + initialColumn] = turn;
                    return 1;
                }
                if (finalRow == initialRow + 1 && finalColumn == initialColumn && target.type == EMPTY)
                    return 1;
               
                if (finalRow == initialRow + 1 && abs(finalColumn - initialColumn) == 1 && target.color == P_WHITE)
                    return 1;
               
                if (initialRow == 4 && abs(finalColumn - initialColumn) == 1 && 
                    turamutaredubla[finalColumn] == turn - 1) 
                {
                    return 1; 
                }
            } 
            else 
            {
                if (initialRow == 6 && finalRow == 4 && finalColumn == initialColumn && 
                    target.type == EMPTY && board[5][initialColumn].type == EMPTY) 
                {
                    turamutaredubla[initialColumn] = turn; 
                    return 1;
                }
                if (finalRow == initialRow - 1 && finalColumn == initialColumn && target.type == EMPTY)
                    return 1;
         
                if (finalRow == initialRow - 1 && abs(finalColumn - initialColumn) == 1 && target.color == P_BLACK)
                    return 1;
                if (initialRow == 3 && abs(finalColumn - initialColumn) == 1 && 
                    turamutaredubla[8 + finalColumn] == turn - 1) 
                {
                    return 1; 
                }
            }
            break;
        case KNIGHT:
            if ((rowDiff == 2 && colDiff == 1) || (rowDiff == 1 && colDiff == 2))
                return 1;
            break;
        case BISHOP:
            if (rowDiff == colDiff && isPathClear(initialColumn, initialRow, finalColumn, finalRow))
                return 1;
            break;
        case ROOK:
            if ((initialRow == finalRow || initialColumn == finalColumn) && isPathClear(initialColumn, initialRow, finalColumn, finalRow))
                return 1;
            break;
        case QUEEN:
            if ((rowDiff == colDiff || initialRow == finalRow || initialColumn == finalColumn) && isPathClear(initialColumn, initialRow, finalColumn, finalRow))
                return 1;
            break;
        case KING:
            if (rowDiff <= 1 && colDiff <= 1)
                return 1;
            break;
        default:
            return 0;
    }
    return 0;
    }

int isInsideBoard(int row, int column) //functie care verifica daca o pozitie e pe tabla
{
    return (row>=0 && row<BOARD_SIZE && column>=0 && column<BOARD_SIZE);
}

int areBoardsEqual(BoardState *b1, BoardState *b2)
{
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            if (b1->board[i][j].type != b2->board[i][j].type || 
                b1->board[i][j].color != b2->board[i][j].color)
                    return 0; //sunt diferite
        }
    }
    return 1; 
}

int countBoardOccurrences(BoardState *newState)//functia care numara de cate ori apare o pozitie in istoric
{
    int count = 0;
    for (int i = 0; i < historyCount; i++)
    {
        if (areBoardsEqual(newState, &history[i]))
            count++;
    }
    return count;
}

int isThreefoldRepetition()//funcția care verifica daca aceeasi poz a aparut de 3 ori
{
    if (historyCount < 3)
        return 0;
    BoardState current = history[historyCount - 1];
    return countBoardOccurrences(&current) >= 3;
}

void checkThreefoldRepetition()//functia care verifica si adaugă poz curenta in istoric
{
    if (historyCount < MAX_MOVES) {
        for (int i = 0; i < BOARD_SIZE; i++)
            for (int j = 0; j < BOARD_SIZE; j++)
                history[historyCount].board[i][j] = board[i][j];
        historyCount++;
    }

    if (isThreefoldRepetition()) 
    {
        printf("Jocul s-a incheiat in remiza(tripla repetare).\n");
        exit(0); 
    }
}

int isSquareAttacked(int row, int column, PieceColor attackerColor)//functie care verifica daca o pozitie este sub atac
{
    //asta e de editat
    
     if (attackerColor == P_BLACK) {
        if (isInsideBoard(row - 1, column - 1) &&
            board[row - 1][column - 1].type == PAWN &&
            board[row - 1][column - 1].color == P_BLACK)
            return 1;
        if (isInsideBoard(row - 1, column + 1) &&
            board[row - 1][column + 1].type == PAWN &&
            board[row - 1][column + 1].color == P_BLACK)
            return 1;
    } else {
        if (isInsideBoard(row + 1, column - 1) &&
            board[row + 1][column - 1].type == PAWN &&
            board[row + 1][column - 1].color == P_WHITE)
            return 1;
        if (isInsideBoard(row + 1, column + 1) &&
            board[row + 1][column + 1].type == PAWN &&
            board[row + 1][column + 1].color == P_WHITE)
            return 1;
    }

    //pana aici  1111
     
    int dr, dc, r, c;
    int knightMoves[8][2] = {{-2, -1}, {-2, 1}, {2, -1}, {2, 1}, 
                             {-1, -2}, {-1, 2}, {1, -2}, {1, 2}};
    for (int i = 0; i < 8; i++) {
        r = row + knightMoves[i][0];
        c = column + knightMoves[i][1];
        if (isInsideBoard(r, c) && board[r][c].type == KNIGHT && board[r][c].color == attackerColor)
            return 1;
    }

    int rookDirections[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    for (int i = 0; i < 4; i++) 
    {
        dr = rookDirections[i][0];
        dc = rookDirections[i][1];
        r = row;
        c = column;
        while (isInsideBoard(r + dr, c + dc)) 
        {
            r =r + dr;
            c =c + dc;
            if (board[r][c].type != EMPTY)
            {
                if ((board[r][c].type == ROOK || board[r][c].type == QUEEN) &&
                    board[r][c].color == attackerColor)
                    return 1;
                break;
            }
        }
    }

    int bishopDirections[4][2] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
    for (int i = 0; i < 4; i++)
    {
        dr = bishopDirections[i][0];
        dc = bishopDirections[i][1];
        r = row;
        c = column;
        while (isInsideBoard(r + dr, c + dc))
        {
            r=r + dr;
            c=c + dc;
            if (board[r][c].type != EMPTY)
            {
                if ((board[r][c].type == BISHOP || board[r][c].type == QUEEN) && board[r][c].color == attackerColor)
                    return 1;
                break;
            }
        }
    }

    int kingMoves[8][2] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, 
                           {0, 1}, {1, -1}, {1, 0}, {1, 1}};
    for (int i = 0; i < 8; i++)
    {
        r = row + kingMoves[i][0];
        c = column + kingMoves[i][1];
        if (isInsideBoard(r, c) && board[r][c].type == KING && board[r][c].color == attackerColor)
            return 1;
    }

    return 0; 
}

int colKingW=4,rowKingW=7; //de schimbat sahu cu astea( sa il punem pe coordonate ca sa nu te poti muta voluntar in sah)
int colKingB=4,rowKingB=0; //de asemenea trb implementat sa nu se atinga regii la sub 1 patrat intre ei


int simulateMoveAndCheck(int fromRow, int fromCol, int toRow, int toCol, PieceColor kingColor) {
    Piece moved = board[fromRow][fromCol];
    Piece captured = board[toRow][toCol];

    board[toRow][toCol] = moved;
    board[fromRow][fromCol] = (Piece){EMPTY, NONE};

    int kingRow, kingCol;
    if (moved.type == KING) {
        kingRow = toRow;
        kingCol = toCol;
    } else {
        // caută poziția regelui pe tablă după mutare
        kingRow = -1;
        kingCol = -1;
        for (int i = 0; i < BOARD_SIZE; i++)
            for (int j = 0; j < BOARD_SIZE; j++)
                if (board[i][j].type == KING && board[i][j].color == kingColor) {
                    kingRow = i;
                    kingCol = j;
                }
    }

    int inCheck = sahInPozitie(kingRow, kingCol, kingColor);

    board[fromRow][fromCol] = moved;
    board[toRow][toCol] = captured;

    return !inCheck;
}

int sahInPozitie(int kingRow, int kingCol, PieceColor kingColor)
{
    PieceColor attackerColor = (kingColor == P_WHITE) ? P_BLACK : P_WHITE;

    // verificare atacuri de pioni
    if (attackerColor == P_BLACK) {
        if (isInsideBoard(kingRow - 1, kingCol - 1) &&
            board[kingRow - 1][kingCol - 1].type == PAWN &&
            board[kingRow - 1][kingCol - 1].color == P_BLACK)
            return 1;
        if (isInsideBoard(kingRow - 1, kingCol + 1) &&
            board[kingRow - 1][kingCol + 1].type == PAWN &&
            board[kingRow - 1][kingCol + 1].color == P_BLACK)
            return 1;
    } else {
        if (isInsideBoard(kingRow + 1, kingCol - 1) &&
            board[kingRow + 1][kingCol - 1].type == PAWN &&
            board[kingRow + 1][kingCol - 1].color == P_WHITE)
            return 1;
        if (isInsideBoard(kingRow + 1, kingCol + 1) &&
            board[kingRow + 1][kingCol + 1].type == PAWN &&
            board[kingRow + 1][kingCol + 1].color == P_WHITE)
            return 1;
    }

    return isSquareAttacked(kingRow, kingCol, attackerColor);
}


int sah(PieceColor kingColor) {
    for (int i = 0; i < BOARD_SIZE; i++)
        for (int j = 0; j < BOARD_SIZE; j++)
            if (board[i][j].type == KING && board[i][j].color == kingColor)
                return sahInPozitie(i, j, kingColor);

    return 0;
}

//start 
int sahmat(PieceColor kingColor) 
{
    if (!sah(kingColor)) 
        return 0; // nu e sah, deci clar nu e sah mat

    PieceColor opponentColor = (kingColor == P_WHITE) ? P_BLACK : P_WHITE;
    int kingRow = -1, kingCol = -1, otherKingRow = -1, otherKingCol = -1;

    for (int i = 0; i < BOARD_SIZE; i++) 
        for (int j = 0; j < BOARD_SIZE; j++) 
            if (board[i][j].type == KING && board[i][j].color == opponentColor) 
            {
                otherKingRow = i;
                otherKingCol = j; 
            }
            
    for (int i = 0; i < BOARD_SIZE; i++) 
        for (int j = 0; j < BOARD_SIZE; j++) 
            if (board[i][j].type == KING && board[i][j].color == kingColor) 
            {
                kingRow = i;
                kingCol = j;
                goto foundKing; 
            }        

    return 1; // nu s-a găsit propriul rege, considerăm sah mat

foundKing:

    // protecție suplimentară
    if (kingRow == -1 || kingCol == -1)
        return 1;

    for (int dx = -1; dx <= 1; dx++) 
    {
        for (int dy = -1; dy <= 1; dy++)
        {
            if (dx == 0 && dy == 0) continue;

            int newRow = kingRow + dx;
            int newCol = kingCol + dy;

            if (isInsideBoard(newRow, newCol)) 
            {            
                if (board[newRow][newCol].color == opponentColor &&
                    !isSquareAttacked(newRow, newCol, opponentColor)) 
                {
                    return 0; 
                }
                if (board[newRow][newCol].color != kingColor &&
                    !(abs(newRow - otherKingRow) <= 1 && abs(newCol - otherKingCol) <= 1)) 
                {   
                    Piece backup = board[newRow][newCol];
                    board[newRow][newCol] = board[kingRow][kingCol];
                    board[kingRow][kingCol] = (Piece){EMPTY, NONE};

                    if (!sah(kingColor)) 
                    {
                        board[kingRow][kingCol] = board[newRow][newCol];
                        board[newRow][newCol] = backup;
                        return 0; 
                    }

                    board[kingRow][kingCol] = board[newRow][newCol];
                    board[newRow][newCol] = backup;
                }
            }
        }
    }

    int attackers[BOARD_SIZE * 2][2];
int attackerCount = 0;

// Găsim toate piesele care atacă regele
for (int i = 0; i < BOARD_SIZE; i++) {
    for (int j = 0; j < BOARD_SIZE; j++) {
        if (board[i][j].color == opponentColor) {
            int turn = (kingColor == P_WHITE) ? 0 : 1;
            if (checkIfMoveIsPossible(j, i, kingCol, kingRow, turn)) {
                attackers[attackerCount][0] = i; // row
                attackers[attackerCount][1] = j; // col
                attackerCount++;
            }
        }
    }
}

if (attackerCount >= 2) {
    printf("Sah mat! Castigator: %s\n", kingColor == P_WHITE ? "Negru" : "Alb");
    return 1; // Nu putem bloca/ucide 2 atacatori simultan => mat
}

int attackerRow = attackers[0][0];
int attackerCol = attackers[0][1];

// Verificăm dacă vreo piesă proprie poate captura atacatorul
for (int i = 0; i < BOARD_SIZE; i++) {
    for (int j = 0; j < BOARD_SIZE; j++) {
        if (board[i][j].color == kingColor) {
            int turn = (kingColor == P_WHITE) ? 0 : 1;
            if (checkIfMoveIsPossible(j, i, attackerCol, attackerRow, turn)) {
                if (simulateMoveAndCheck(i, j, attackerRow, attackerCol, kingColor)) {
                    return 0; // O piesă poate captura atacatorul și îl scoate pe rege din șah
                }
            }
        }
    }
}

// Dacă atacatorul nu e cal sau pion, putem încerca să blocăm atacul
if (board[attackerRow][attackerCol].type != KNIGHT && board[attackerRow][attackerCol].type != PAWN) {
    int dx = (kingCol - attackerCol) != 0 ? (kingCol - attackerCol) / abs(kingCol - attackerCol) : 0;
    int dy = (kingRow - attackerRow) != 0 ? (kingRow - attackerRow) / abs(kingRow - attackerRow) : 0;

    int blockRow = attackerRow + dy;
    int blockCol = attackerCol + dx;

    while (blockRow != kingRow || blockCol != kingCol) {
        if (!isInsideBoard(blockRow, blockCol))
            break;

        for (int i = 0; i < BOARD_SIZE; i++) 
        {
            for (int j = 0; j < BOARD_SIZE; j++) 
            {
                if (board[i][j].color == kingColor && board[i][j].type != KING) 
                {
                   for (int toRow = 0; toRow < BOARD_SIZE; toRow++) 
                    {
                        for (int toCol = 0; toCol < BOARD_SIZE; toCol++) 
                        {
                            int turn=(kingColor==P_WHITE)?0:1;
                       if (checkIfMoveIsPossible(j, i, toCol, toRow, turn))

                        {

                            Piece moved = board[i][j];
                            Piece captured = board[toRow][toCol];

                            board[toRow][toCol] = moved;
                            board[i][j] = (Piece){EMPTY, NONE};

                            int stillInCheck = sah(kingColor);
                            board[i][j] = moved;
                            board[toRow][toCol] = captured;

                            if (!stillInCheck) 
                                return 0;
                            }
                        }
                    }
                }
            }
        }

        blockRow += dy;
        blockCol += dx;
    }
}

printf("Sah mat! Castigator: %s\n", kingColor == P_WHITE ? "Negru" : "Alb");
return 1; // Nicio mutare nu poate salva regele => șah mat
}

void saveGameToFile(const char *filename)
{
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Eroare la deschiderea fisierului pentru salvare");
        return;
    }

    fprintf(file, "    A B C D E F G H\n");
    fprintf(file, "    - - - - - - - -\n");
    for (int i = 0; i < BOARD_SIZE; i++) {
        fprintf(file, "%d | ", 8 - i);
        for (int j = 0; j < BOARD_SIZE; j++) {
            Piece p = board[i][j];
            char symbol = '.';
            if (p.type != EMPTY) {
                switch (p.type) 
                {
                    case PAWN:   symbol = 'p'; break;
                    case KNIGHT: symbol = 'n'; break;
                    case BISHOP: symbol = 'b'; break;
                    case ROOK:   symbol = 'r'; break;
                    case QUEEN:  symbol = 'q'; break;
                    case KING:   symbol = 'k'; break;
                    default: break;
                }
                if (p.color == P_WHITE) 
                {
                    symbol = toupper(symbol);
                }
            } else {
                symbol = ((i + j) % 2 == 0) ? 'O' : '#';
            }
            fprintf(file, "%c ", symbol);
        }
        fprintf(file, "| %d\n", 8 - i);
    }
    fprintf(file, "    - - - - - - - -\n");
    fprintf(file, "    A B C D E F G H\n");

    fclose(file);
    printf("Jocul a fost salvat in fisierul '%s'.\n", filename);
}

int kingWmoves=0, kingBmoves=0, rook1Wmoves=0,rook2Wmoves=0,rook1Bmoves=0,rook2Bmoves=0; //miscarile ceor 2 regi si 4 ture

int castle(int rookColumn, PieceColor color)//functia pentru rocada
{
    int kingRow=(color==P_WHITE) ? 7 : 0;
    unsigned int kingColumn=4;
    PieceColor opponentColor=(color==P_WHITE)?P_BLACK:P_WHITE;

    if ((color==P_WHITE && (kingWmoves!=0)) || (color==P_BLACK && (kingBmoves!=0)))
        return 0;
        
    if (rookColumn == 0 && ((color == P_WHITE && rook1Wmoves) || (color == P_BLACK && rook1Bmoves)))
        return 0;
        
    if (rookColumn == 7 && ((color == P_WHITE && rook2Wmoves) || (color == P_BLACK && rook2Bmoves)))
        return 0;

    if (board[kingRow][rookColumn].type != ROOK || board[kingRow][rookColumn].color != color)
        return 0;

    if (!isPathClear(kingColumn, kingRow, rookColumn, kingRow))
        return 0;

    if (isSquareAttacked(kingRow, kingColumn, opponentColor)||
    isSquareAttacked(kingRow, (rookColumn == 0) ? 3 : 5, opponentColor)||
    isSquareAttacked(kingRow, (rookColumn == 0) ? 2 : 6, opponentColor) ) 
    {
        return 0;
    }
    
    if (rookColumn == 0)
    {
        board[kingRow][2] = board[kingRow][kingColumn]; 
        board[kingRow][kingColumn] = (Piece){EMPTY, NONE}; 
        board[kingRow][3] = board[kingRow][rookColumn]; 
        board[kingRow][rookColumn] = (Piece){EMPTY, NONE}; 
    }
    else if (rookColumn == 7)
    {
        board[kingRow][6] = board[kingRow][kingColumn]; 
        board[kingRow][kingColumn] = (Piece){EMPTY, NONE};
        board[kingRow][5] = board[kingRow][rookColumn];
        board[kingRow][rookColumn] = (Piece){EMPTY, NONE};
    }

    if (color==P_WHITE) 
    {
        kingWmoves=1;
        if (rookColumn==0) 
            rook1Wmoves=1;
        if (rookColumn==7) 
            rook2Wmoves=1;
    } 
    else 
    {
        kingBmoves=1;
        if (rookColumn==0) 
            rook1Bmoves=1;
        if (rookColumn==7) 
            rook2Bmoves=1;
    }
    return 1;
}

int stalemate(PieceColor playerColor, int turn) 
{
    if (sah(playerColor))
        return 0;

    for (int fromRow = 0; fromRow < BOARD_SIZE; fromRow++) 
    {
        for (int fromCol = 0; fromCol < BOARD_SIZE; fromCol++) 
        {
            if (board[fromRow][fromCol].color == playerColor) 
            {
                for (int toRow = 0; toRow < BOARD_SIZE; toRow++) 
                {
                    for (int toCol = 0; toCol < BOARD_SIZE; toCol++) 
                    {
                        if (checkIfMoveIsPossible(fromCol, fromRow, toCol, toRow, (playerColor == P_WHITE) ? 0 : 1)) 
                        {
                            Piece moved = board[fromRow][fromCol];
                            Piece captured = board[toRow][toCol];

                            board[toRow][toCol] = moved;
                            board[fromRow][fromCol] = (Piece){EMPTY, NONE};

                            int stillInCheck = sah(playerColor);

                            board[fromRow][fromCol] = moved;
                            board[toRow][toCol] = captured;

                            if (!stillInCheck) 
                                return 0;
                        }

                    }
                }
            }
        }
    }
    return 1; 
}

int turnspiecesnottaken=0; //variabila pt regula cu 50 de ture fara a fi luata o piesa
