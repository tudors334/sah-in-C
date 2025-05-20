#ifndef SAH_AI_H
#define SAH_AI_H

#include "logica_sah.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int fromRow, fromCol;
    int toRow, toCol;
    Piece captured;
} AIMove;

int pieceValue(PieceType type);
int evaluateBoard(PieceColor color);
int generateAllMoves(PieceColor color, AIMove moves[], int turn);
int minimax(int depth, int maximizing, PieceColor aiColor, int turn);
void handleAIMove(PieceColor aiColor);
void handleRandomAIMove(PieceColor aiColor);

#endif
