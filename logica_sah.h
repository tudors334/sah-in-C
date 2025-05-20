#ifndef LOGICA_SAH_H
#define LOGICA_SAH_H

#define MAX_MOVES 1000
#define BOARD_SIZE 8

typedef enum { EMPTY=0, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING } PieceType;
typedef enum { NONE=0, P_WHITE, P_BLACK } PieceColor;

typedef struct {
    PieceType type;
    PieceColor color;
} Piece;

typedef struct {
    int fromRow, fromCol;
    int toRow, toCol;
} Move;

typedef struct {
    Piece board[BOARD_SIZE][BOARD_SIZE];
} BoardState;

extern Piece board[BOARD_SIZE][BOARD_SIZE];
extern int turnspiecesnottaken;
extern BoardState history[MAX_MOVES];
extern int historyCount;
extern int turamutaredubla[16];
extern int colKingW, rowKingW;
extern int colKingB, rowKingB;
extern int kingWmoves, kingBmoves, rook1Wmoves,rook2Wmoves,rook1Bmoves,rook2Bmoves; 

void initBoard(void);
void printBoard(void);
int isSquareAttacked(int row, int column, PieceColor attackerColor);
int checkIfMoveIsPossible(int initialCol, int initialRow, int finalCol, int finalRow, int turn);
int simulateMoveAndCheck(int fromRow, int fromCol, int toRow, int toCol, PieceColor kingColor);
int sahInPozitie(int kingRow, int kingCol, PieceColor kingColor);
int sah(PieceColor kingColor);
int sahmat(PieceColor kingColor);
int castle(int rookColumn, PieceColor color);
int isInsideBoard(int row, int column);
int stalemate(PieceColor playerColor, int turn);
int areBoardsEqual(BoardState *b1, BoardState *b2);
int countBoardOccurrences(BoardState *newState);
void checkThreefoldRepetition(void);
int isThreefoldRepetition();
void saveGameToFile(const char *filename);

#endif 