//acesta nu este main-ul proiectului, ci doar primul fisier adaugat pe github in martie si am uitat sa il redenumesc, scuze de neintelegere

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

#define BOARD_SIZE 8

typedef enum { EMPTY=0, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING } PieceType;
typedef enum { NONE=0, WHITE, BLACK } PieceColor;

typedef struct {
    PieceType type;
    PieceColor color;
} Piece;

Piece board[BOARD_SIZE][BOARD_SIZE];

void initBoard() {
    for (int i = 0; i < BOARD_SIZE; i++)
        for (int j = 0; j < BOARD_SIZE; j++)
            board[i][j] = (Piece){EMPTY, NONE};
    board[0][0] = board[0][7] = (Piece){ROOK, WHITE};
    board[0][1] = board[0][6] = (Piece){KNIGHT, WHITE};
    board[0][2] = board[0][5] = (Piece){BISHOP, WHITE};
    board[0][3] = (Piece){QUEEN, WHITE};
    board[0][4] = (Piece){KING, WHITE};
    for (int j = 0; j < BOARD_SIZE; j++)
        board[1][j] = (Piece){PAWN, WHITE};

    board[7][0] = board[7][7] = (Piece){ROOK, BLACK};
    board[7][1] = board[7][6] = (Piece){KNIGHT, BLACK};
    board[7][2] = board[7][5] = (Piece){BISHOP, BLACK};
    board[7][3] = (Piece){QUEEN, BLACK};
    board[7][4] = (Piece){KING, BLACK};
    for (int j = 0; j < BOARD_SIZE; j++)
        board[6][j] = (Piece){PAWN, BLACK};
}

void printBoard() {
    char symbol;
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            Piece p = board[i][j];
            if (p.type == EMPTY) 
            {
                // Alternăm pătratele albe și negre pentru efect vizual
                if ((i + j) % 2 == 0)
                    symbol = 'O'; // Alb
                else
                    symbol = '#'; // Negru
            }
            else if (p.type != EMPTY) {
                switch (p.type) {
                    case PAWN:   symbol = 'P'; break;
                    case KNIGHT: symbol = 'N'; break;
                    case BISHOP: symbol = 'B'; break;
                    case ROOK:   symbol = 'R'; break;
                    case QUEEN:  symbol = 'Q'; break;
                    case KING:   symbol = 'K'; break;
                    default: break;
                }
                if (p.color == BLACK)
                    symbol += 32;
            }
            printf("%c ", symbol);
        }
        printf("\n");
    }
}

int isPathClear(int initialColumn, int initialRow, int finalColumn, int finalRow) {
    int rowStep = (finalRow > initialRow) ? 1 : (finalRow < initialRow) ? -1 : 0;
    int colStep = (finalColumn > initialColumn) ? 1 : (finalColumn < initialColumn) ? -1 : 0;
    
    int r = initialRow + rowStep;
    int c = initialColumn + colStep;
    
    while (r != finalRow || c != finalColumn) {
        if (board[r][c].type != EMPTY)
            return 0;
        r += rowStep;
        c += colStep;
    }
    return 1;
}

int checkIfMoveIsPossible(int initialColumn, int initialRow, int finalColumn, int finalRow) {
    Piece piece = board[initialRow][initialColumn];
    if (piece.type == EMPTY)
        return 0;
    Piece target = board[finalRow][finalColumn];
    if (target.color == piece.color)
        return 0;
    
    int rowDiff = abs(finalRow - initialRow);
    int colDiff = abs(finalColumn - initialColumn);
    
    switch (piece.type) {
        case PAWN:
            if (piece.color == WHITE) {
                if (finalRow == initialRow + 1 && finalColumn == initialColumn && target.type == EMPTY)
                    return 1;
                if (finalRow == initialRow + 1 && abs(finalColumn - initialColumn) == 1 && target.color == BLACK)
                    return 1;
            } else {
                if (finalRow == initialRow - 1 && finalColumn == initialColumn && target.type == EMPTY)
                    return 1;
                if (finalRow == initialRow - 1 && abs(finalColumn - initialColumn) == 1 && target.color == WHITE)
                    return 1;
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

void displayPossibleMoves(int column, int row) {
    Piece piece = board[row][column];
    if (piece.type == EMPTY) {
        printf("No piece at the given position.\n");
        return;
    }
    printf("Possible moves for %c at (%d, %d):\n", piece.color == WHITE ? 'W' : 'B', row, column);
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (checkIfMoveIsPossible(column, row, j, i))
                printf("(%d, %d)\n", i, j);
        }
    }
}
int sah(PieceColor kingColor) {
    int kingRow, kingCol;
    for (int i = 0; i < BOARD_SIZE; i++)
        for (int j = 0; j < BOARD_SIZE; j++)
            if (board[i][j].type == KING && board[i][j].color == kingColor)
                kingRow = i, kingCol = j;

    for (int i = 0; i < BOARD_SIZE; i++)
        for (int j = 0; j < BOARD_SIZE; j++)
            if (board[i][j].color != kingColor && board[i][j].color != NONE)
                if (checkIfMoveIsPossible(j, i, kingCol, kingRow))
                    return 1;
    return 0;
}

int sahmat(PieceColor kingColor) {
    if (!sah(kingColor)) return 0;
    
    for (int i = 0; i < BOARD_SIZE; i++)
        for (int j = 0; j < BOARD_SIZE; j++)
            if (board[i][j].color == kingColor) 
            {
                for (int x = 0; x < BOARD_SIZE; x++)
                    for (int y = 0; y < BOARD_SIZE; y++) 
                    {
                        Piece temp = board[x][y];
                        if (checkIfMoveIsPossible(j, i, y, x)) 
                        {
                            board[x][y] = board[i][j];
                            board[i][j] = (Piece){EMPTY, NONE};
                            int stillCheck = sah(kingColor);
                            board[i][j] = board[x][y];
                            board[x][y] = temp;
                            if (!stillCheck) return 0;
                        }
                    }
            }
    printf("Sah mat! Castigator: %s\n", kingColor == WHITE ? "Negru" : "Alb");
    return 1;
}
void play() {
    int turn = 0;
    while (1) {
        printBoard();
        int initialColumn, initialRow, finalColumn, finalRow;
        PieceColor currentPlayer = (turn % 2 == 0) ? WHITE : BLACK;

        printf("Jucătorul %s, alegeți o mișcare (de exemplu, '3,1 3,3'):\n", currentPlayer == WHITE ? "Alb" : "Negru");

        // Use the new input format '3,1 3,3'
        char move[10];
        fgets(move, sizeof(move), stdin);

        // Parse the input in the new format
        if (sscanf(move, "%d,%d %d,%d", &initialRow, &initialColumn, &finalRow, &finalColumn) != 4) {
            printf("Input invalid. Încercați din nou.\n");
            continue;
        }

        // Convert to 0-based indexing for internal board
        initialRow = BOARD_SIZE - initialRow;
        initialColumn -= 1;  // Fix column conversion
        finalRow = BOARD_SIZE - finalRow;
        finalColumn -= 1;  // Fix column conversion

        // Check if the move is valid
        if (checkIfMoveIsPossible(initialColumn, initialRow, finalColumn, finalRow)) {
            board[finalRow][finalColumn] = board[initialRow][initialColumn];
            board[initialRow][initialColumn] = (Piece){EMPTY, NONE}; // Remove piece from initial position

            if (sahmat(currentPlayer)) {
                break; // If checkmate occurs, end the game
            }
        } else {
            printf("Mișcare invalidă. Încercați din nou.\n");
            continue;
        }
        turn++; // Change turn
    }
}
int main() {
    initBoard();
    //printBoard();
    //printf("\nTesting possible moves for a piece:\n");
    //displayPossibleMoves(1, 1);  Example: Rook at position (0,0)
    play();
    return 0;
}
