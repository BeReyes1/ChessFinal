#pragma once

#include "Game.h"
#include "Grid.h"
#include "Bitboard.h"

constexpr int pieceSize = 80;

enum ChessPiece
{
    NoPiece,
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King
};

class Chess : public Game
{
public:
    Chess();
    ~Chess();

    void setUpBoard() override;

    bool canBitMoveFrom(Bit &bit, BitHolder &src) override;
    bool canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst) override;
    bool actionForEmptyHolder(BitHolder &holder) override;

    void stopGame() override;

    Player *checkForWinner() override;
    bool checkForDraw() override;

    std::string initialStateString() override;
    std::string stateString() override;
    void setStateString(const std::string &s) override;

    Grid* getGrid() override { return _grid; }

private:
    Bit* PieceForPlayer(const int playerNumber, ChessPiece piece);
    Player* ownerAt(int x, int y) const;
    void FENtoBoard(const std::string& fen);
    char pieceNotation(int x, int y) const;

    int indexOfHolder(BitHolder& holder);
    void bitMovedFromTo(Bit &bit, BitHolder &src, BitHolder &dst) override;

    void syncGridFromBitboards();
    uint64_t _whiteBoard = 0ULL;
    uint64_t _blackBoard = 0ULL;


    uint64_t _pieces[2][7]; 

    uint64_t whiteOccupancy() const;
    uint64_t blackOccupancy() const;
    uint64_t allOccupancy() const;

    BitboardElement knightMoves(BitboardElement knights);
    BitboardElement kingMoves(BitboardElement king);
    BitboardElement pawnMoves(BitboardElement pawns, BitboardElement empty, BitboardElement enemy, bool isWhite);

    Grid* _grid;
};