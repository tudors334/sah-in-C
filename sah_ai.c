#include "sah_ai.h"
#include "logica_sah.h"
#include <stdlib.h>
#include <time.h>


int pieceValue(PieceType type) // valoare piese
{
    switch (type) {
        case PAWN: return 1;
        case KNIGHT: return 3;
        case BISHOP: return 3;
        case ROOK: return 5;
        case QUEEN: return 9;
        case KING: return 100;
        default: return 0;
    }
}

int evaluateBoard(PieceColor color) // valoarea pieselor unui jucator
{
    int score = 0;
    for (int i = 0; i < BOARD_SIZE; i++)
        for (int j = 0; j < BOARD_SIZE; j++)
            {
            Piece p = board[i][j];
            if (p.type != EMPTY)
            {
                int val = pieceValue(p.type);
                score += (p.color == color) ? val : -val;
            }
        }
    return score;
}

int generateAllAIMoves(PieceColor color, AIMove moves[], int turn) 
{
    int count = 0;

    // Mutări normale (și capturi)
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            if (board[i][j].color == color)
            {
                for (int r = 0; r < BOARD_SIZE; r++)
                {
                    for (int c = 0; c < BOARD_SIZE; c++)
                    {
                        if (checkIfMoveIsPossible(j, i, c, r, turn))
                        {
                            moves[count++] = (AIMove){i, j, r, c, board[r][c]};
                        }
                    }
                }

                Piece p = board[i][j];

                // === EN PASSANT ===
                if (p.type == PAWN)
                {
                    int dir = (color == P_WHITE) ? -1 : 1;
                    if (i == ((color == P_WHITE) ? 3 : 4))  // randul unde e posibil en passant
                    {
                        // stânga
                        if (j > 0 && board[i][j -1].type == PAWN && board[i][j -1].color != color &&
                            board[i -dir][j -1].type == EMPTY)
                        {
                            moves[count++] = (AIMove){i, j, i + dir, j -1, board[i][j -1]};
                        }
                        // dreapta
                        if (j < BOARD_SIZE -1 && board[i][j +1].type == PAWN && board[i][j +1].color != color &&
                            board[i -dir][j +1].type == EMPTY)
                        {
                            moves[count++] = (AIMove){i, j, i + dir, j +1, board[i][j +1]};
                        }
                    }
                }

                // === ROCADE ===
                if (p.type == KING)
                {
                    // Rocadă scurtă (dreapta)
                    if (castle(color, 1))  // 1 = scurtă
                    {
                        moves[count++] = (AIMove){i, j, i, j +2, board[i][j +2]};
                    }
                    // Rocadă lungă (stânga)
                    if (castle(color, 7))  // 0 = lungă
                    {
                        moves[count++] = (AIMove){i, j, i, j -2, board[i][j -2]};
                    }
                }
            }
        }
    }
    return count;
}


int minimax(int depth, int maximizing, PieceColor aiColor, int turn) // algoritm minimax
{
    if (depth == 0)
        return evaluateBoard(aiColor);

    AIMove moves[256];
    PieceColor currentColor = (maximizing) ? aiColor : (aiColor == P_WHITE ? P_BLACK : P_WHITE);
    int count = generateAllAIMoves(currentColor, moves, turn);

    if (count == 0)
        return evaluateBoard(aiColor); // fără mutări

    int best = maximizing ? -100000 : 100000;

    for (int i = 0; i < count; i++) {
        AIMove m = moves[i];
        Piece moved = board[m.fromRow][m.fromCol];
        Piece captured = board[m.toRow][m.toCol];

        board[m.toRow][m.toCol] = moved;
        board[m.fromRow][m.fromCol] = (Piece){EMPTY, NONE};

        int val = minimax(depth - 1, !maximizing, aiColor, 1 - turn);

        board[m.fromRow][m.fromCol] = moved;
        board[m.toRow][m.toCol] = captured;

        best = maximizing ? (val > best ? val : best)
                          : (val < best ? val : best);
    }

    return best;
}

void handleAIMove(PieceColor aiColor) 
{
    int bestValue = -100000;
    AIMove moves[256];
    AIMove bestMove;
    int count = generateAllAIMoves(aiColor, moves, (aiColor == P_WHITE ? 0 : 1));

    if (count == 0) return;

    for (int i = 0; i < count; i++) 
    {
        AIMove m = moves[i];
        Piece moved = board[m.fromRow][m.fromCol];
        Piece captured = board[m.toRow][m.toCol];

        board[m.toRow][m.toCol] = moved;
        board[m.fromRow][m.fromCol] = (Piece){EMPTY, NONE};

        int skip = sah(aiColor);  // verifică dacă AI-ul rămâne în sah

        int value = -100000;
        if (!skip)
            value = minimax(2, 0, aiColor, (aiColor == P_WHITE ? 1 : 0)); // adâncime 2

        board[m.fromRow][m.fromCol] = moved;
        board[m.toRow][m.toCol] = captured;

        if (!skip && value > bestValue) {
            bestValue = value;
            bestMove = m;
        }
    }

    if (bestValue == -100000) return;  // nici o mutare legală

    Piece moved = board[bestMove.fromRow][bestMove.fromCol];
    Piece captured = board[bestMove.toRow][bestMove.toCol];

    // En passant
    if (moved.type == PAWN && bestMove.fromCol != bestMove.toCol && captured.type == EMPTY)
    {
        int capRow = (moved.color == P_WHITE) ? bestMove.toRow +1 : bestMove.toRow -1;
        if (capRow >=0 && capRow < BOARD_SIZE)
            board[capRow][bestMove.toCol] = (Piece){EMPTY, NONE};
    }

    // Mutare finală
    board[bestMove.toRow][bestMove.toCol] = moved;
    board[bestMove.fromRow][bestMove.fromCol] = (Piece){EMPTY, NONE};

    // Promovare pion
    if (moved.type == PAWN && (bestMove.toRow == 0 || bestMove.toRow == BOARD_SIZE -1)) {
        board[bestMove.toRow][bestMove.toCol].type = QUEEN;
    }

    // Rocadă
    if (moved.type == KING && abs(bestMove.toCol - bestMove.fromCol) == 2) {
        if (bestMove.toCol == 6) {
            board[bestMove.toRow][5] = board[bestMove.toRow][7];
            board[bestMove.toRow][7] = (Piece){EMPTY, NONE};
        }
        else if (bestMove.toCol == 2) {
            board[bestMove.toRow][3] = board[bestMove.toRow][0];
            board[bestMove.toRow][0] = (Piece){EMPTY, NONE};
        }
    }
}

void handleRandomAIMove(PieceColor aiColor)
{
    AIMove moves[256];
    int count = generateAllAIMoves(aiColor, moves, (aiColor == P_WHITE) ? 0 : 1);

    if (count == 0) return;

    srand(time(NULL));

    int legalMoves[256];
    int legalCount = 0;

    // Filtrăm doar mutările care nu lasă regele în sah
    for (int i = 0; i < count; i++) {
        AIMove m = moves[i];
        Piece moved = board[m.fromRow][m.fromCol];
        Piece captured = board[m.toRow][m.toCol];

        board[m.toRow][m.toCol] = moved;
        board[m.fromRow][m.fromCol] = (Piece){EMPTY, NONE};

        if (!sah(aiColor))
            legalMoves[legalCount++] = i;

        board[m.fromRow][m.fromCol] = moved;
        board[m.toRow][m.toCol] = captured;
    }

    if (legalCount == 0) return;

    int index = legalMoves[rand() % legalCount];
    AIMove m = moves[index];

    Piece moved = board[m.fromRow][m.fromCol];
    Piece captured = board[m.toRow][m.toCol];

    // En passant
    if (moved.type == PAWN && m.fromCol != m.toCol && captured.type == EMPTY)
    {
        if (moved.color == P_WHITE)
            board[m.toRow +1][m.toCol] = (Piece){EMPTY, NONE};
        else
            board[m.toRow -1][m.toCol] = (Piece){EMPTY, NONE};
    }

    board[m.toRow][m.toCol] = moved;
    board[m.fromRow][m.fromCol] = (Piece){EMPTY, NONE};

    // Promovare pion
    if (moved.type == PAWN && (m.toRow == 0 || m.toRow == BOARD_SIZE -1))
        board[m.toRow][m.toCol].type = QUEEN;

    // Rocadă
    if (moved.type == KING && abs(m.toCol - m.fromCol) == 2)
    {
        if (m.toCol == 6)
        {
            board[m.toRow][5] = board[m.toRow][7];
            board[m.toRow][7] = (Piece){EMPTY, NONE};
        }
        else if (m.toCol == 2)
        {
            board[m.toRow][3] = board[m.toRow][0];
            board[m.toRow][0] = (Piece){EMPTY, NONE};
        }
    }
}

