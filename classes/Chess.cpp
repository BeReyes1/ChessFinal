#include "Chess.h"
#include <limits>
#include <cmath>
#include "Bitboard.h"
#include "MagicBitboards.h"

Chess::Chess()
{
    _grid = new Grid(8, 8);
    memset(_pieces, 0, sizeof(_pieces));
    initMagicBitboards();
}

Chess::~Chess()
{
    delete _grid;
}

uint64_t Chess::whiteOccupancy() const
{
    return _pieces[0][Pawn] |
           _pieces[0][Knight] |
           _pieces[0][King] |
           _pieces[0][Bishop] |
           _pieces[0][Rook] |
           _pieces[0][Queen];
}

uint64_t Chess::blackOccupancy() const
{
    return _pieces[1][Pawn] |
           _pieces[1][Knight] |
           _pieces[1][King] |
           _pieces[1][Bishop] |
           _pieces[1][Rook] |
           _pieces[1][Queen];
}

uint64_t Chess::allOccupancy() const
{
    return whiteOccupancy() | blackOccupancy();
}

char Chess::pieceNotation(int x, int y) const
{
    const char *whitepieces = { "0PNBRQK" };
    const char *blackpieces = { "0pnbrqk" };
    Bit *bit = _grid->getSquare(x, y)->bit();
    char notation = '0';
    if (bit) 
    {
        notation = bit->gameTag() < 128 ? whitepieces[bit->gameTag()] 
        : blackpieces[bit->gameTag()-128];
    }
    return notation;
}

Bit* Chess::PieceForPlayer(const int playerNumber, ChessPiece piece)
{
    const char* pieces[] = { "pawn.png", "knight.png", "bishop.png", "rook.png", "queen.png", "king.png" };

    Bit* bit = new Bit();
    // should possibly be cached from player class?
    const char* pieceName = pieces[piece - 1];
    std::string spritePath = std::string("") + (playerNumber == 0 ? "w_" : "b_") + pieceName;
    bit->LoadTextureFromFile(spritePath.c_str());
    bit->setOwner(getPlayerAt(playerNumber));
    bit->setSize(pieceSize, pieceSize);

    return bit;
}

void Chess::setUpBoard()
{
    setNumberOfPlayers(2);
    _gameOptions.rowX = 8;
    _gameOptions.rowY = 8;

    _grid->initializeChessSquares(pieceSize, "boardsquare.png");
    FENtoBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");

    startGame();
}

void Chess::FENtoBoard(const std::string& fen) {
    // convert a FEN string to a board
    // FEN is a space delimited string with 6 fields
    // 1: piece placement (from white's perspective)
    // NOT PART OF THIS ASSIGNMENT BUT OTHER THINGS THAT CAN BE IN A FEN STRING
    // ARE BELOW
    // 2: active color (W or B)
    // 3: castling availability (KQkq or -)
    // 4: en passant target square (in algebraic notation, or -)
    // 5: halfmove clock (number of halfmoves since the last capture or pawn advance

    memset(_pieces, 0, sizeof(_pieces));

    int x = 0;
    int y = 0;

    const std::string pieceOrder = "pnbrqk";

    for (char c : fen)
    {
        if (c == ' ') break;

        if (c == '/')
        {
            y++;
            x = 0;
            continue;
        }

        if (isdigit(c))
        {
            x += c - '0';
            continue;
        }

        int playerNumber = isupper(c) ? 0 : 1;
        char lower = tolower(c);
        size_t index = pieceOrder.find(lower);

        if (index == std::string::npos) continue;

        ChessPiece piece = static_cast<ChessPiece>(index + 1);
        int square = (7 - y) * 8 + x;
        _pieces[playerNumber][piece] |= (1ULL << square);
        x++;
    }

    syncGridFromBitboards();
}

void Chess::syncGridFromBitboards()
{
    _grid->forEachSquare([](ChessSquare* s, int, int)
    {
        s->setBit(nullptr);
    });

    for (int color = 0; color < 2; color++)
    {
        for (int piece = 1; piece <= 6; piece++)
        {
           BitboardElement board(_pieces[color][piece]);

            board.forEachBit([&](int sq)
            {
                int x = sq % 8;
                int y = sq / 8;

                Bit* bit = PieceForPlayer(color, (ChessPiece)piece);
                ChessSquare* square = _grid->getSquare(x, y);

                bit->setPosition(square->getPosition());
                bit->setParent(square);
                bit->setGameTag(color == 0 ? piece : piece + 128);

                square->setBit(bit);
            });
        }
    }
}

BitboardElement Chess :: knightMoves(BitboardElement knights) 
{
    constexpr uint64_t FILE_A = 0x0101010101010101ULL;
    constexpr uint64_t FILE_H = 0x8080808080808080ULL;

    uint64_t k = knights.getData();

    uint64_t left1 = (k >> 1) & ~FILE_H;
    uint64_t left2 = (k >> 2) & ~(FILE_H | (FILE_H >> 1));
    uint64_t right1 = (k << 1) & ~FILE_A;
    uint64_t right2 = (k << 2) & ~(FILE_A | (FILE_A << 1));

    uint64_t h1 = left2 | right2;
    uint64_t h2 = left1 | right1;

    uint64_t moves = (h1 << 8) | (h1 >> 8) | (h2 << 16) | (h2 >> 16);

    return BitboardElement(moves);
}

BitboardElement Chess :: kingMoves(BitboardElement king) 
{
    constexpr uint64_t FILE_A = 0x0101010101010101ULL;
    constexpr uint64_t FILE_H = 0x8080808080808080ULL;

    uint64_t k = king.getData();

    uint64_t attacks =
        (k << 8) | (k >> 8) |
        ((k << 1) & ~FILE_A) |
        ((k >> 1) & ~FILE_H) |
        ((k << 9) & ~FILE_A) |
        ((k << 7) & ~FILE_H) |
        ((k >> 9) & ~FILE_H) |
        ((k >> 7) & ~FILE_A);

    return BitboardElement(attacks);
}

BitboardElement Chess :: pawnMoves(BitboardElement pawns, BitboardElement empty, BitboardElement enemy, bool isWhite) 
{
    constexpr uint64_t FILE_A = 0x0101010101010101ULL;
    constexpr uint64_t FILE_H = 0x8080808080808080ULL;

    uint64_t p = pawns.getData();
    uint64_t e = empty.getData();
    uint64_t opp = enemy.getData();

    uint64_t moves = 0;
   
    uint64_t one = (isWhite) ? (p << 8) & e : (p >> 8) & e;
    uint64_t rank = (isWhite) ? 0x000000000000FF00ULL : 0x00FF000000000000ULL;
    uint64_t two = (isWhite) ? ((p & rank) << 8) & e : ((p & rank) >> 8) & e;
    two = (isWhite) ? (two << 8) & e : (two >> 8) & e;

    uint64_t left = (isWhite) ? ((p & ~FILE_A) << 7) & opp : ((p & ~FILE_H) >> 7) & opp;
    uint64_t right = (isWhite) ? ((p & ~FILE_H) << 9) & opp : ((p & ~FILE_A) >> 9) & opp;

    moves = one | two | left | right;

    return BitboardElement(moves);
}

int Chess::indexOfHolder(BitHolder& holder)
{   
    ChessSquare* square = dynamic_cast<ChessSquare*>(&holder);
    if (!square) return -1;

    return square->getRow() * 8 + square->getColumn();
}


bool Chess::actionForEmptyHolder(BitHolder &holder)
{
    return false;
}

bool Chess::canBitMoveFrom(Bit &bit, BitHolder &src)
{
    // need to implement friendly/unfriendly in bit so for now this hack
    int currentPlayer = getCurrentPlayer()->playerNumber() * 128;
    int pieceColor = bit.gameTag() & 128;
    if (pieceColor == currentPlayer) return true;
    return false;
}

bool Chess::canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst)
{
    int from = indexOfHolder(src);
    int to = indexOfHolder(dst);
    if (from < 0 || to < 0) 
    {
        return false;
    }

    int tag = bit.gameTag();
    int color = (tag & 128) ? 1 : 0;
    ChessPiece piece = static_cast<ChessPiece>(tag & 127);

    if (color != getCurrentPlayer()->playerNumber()) return false;

    uint64_t fromMask = 1ULL << from;
    uint64_t toMask = 1ULL << to;

    uint64_t occupancy = allOccupancy();
    uint64_t friendly = (color == 0) ? whiteOccupancy() : blackOccupancy();
    uint64_t enemy = (color == 0) ? blackOccupancy() : whiteOccupancy();

    uint64_t moves = 0ULL;

    switch (piece)
    {
        case Pawn:
            moves = pawnMoves(
                BitboardElement(fromMask),
                BitboardElement(~occupancy),
                BitboardElement(enemy),
                color == 0).getData();
            break;

        case Knight:
            moves = knightMoves(BitboardElement(fromMask)).getData();
            break;

        case King:
            moves = kingMoves(BitboardElement(fromMask)).getData();
            break;

        case Bishop:
            moves = getBishopAttacks(from, occupancy);
            break;

        case Rook:
            moves = getRookAttacks(from, occupancy);
            break;

        case Queen:
            moves = getQueenAttacks(from, occupancy);
            break;

        default:
            return false;
    }

    moves &= ~friendly;

    return (moves & toMask) != 0;
}

void Chess::bitMovedFromTo(Bit &bit, BitHolder &src, BitHolder &dst)
{
    int from = indexOfHolder(src);
    int to = indexOfHolder(dst);

    int tag = bit.gameTag();
    int color = (tag & 128) ? 1 : 0;
    ChessPiece piece = static_cast<ChessPiece>(tag & 127);

    uint64_t fromMask = 1ULL << from;
    uint64_t toMask = 1ULL << to;

    _pieces[color][piece] &= ~fromMask;

    for (int p = 1; p <= 6; p++)
    {
        _pieces[1-color][p] &= ~toMask;
    }
    _pieces[color][piece] |= toMask;

    Game::bitMovedFromTo(bit, src, dst);
}

void Chess::stopGame()
{
    _grid->forEachSquare([](ChessSquare* square, int x, int y) {
        square->destroyBit();
    });
}

Player* Chess::ownerAt(int x, int y) const
{
    if (x < 0 || x >= 8 || y < 0 || y >= 8) {
        return nullptr;
    }

    auto square = _grid->getSquare(x, y);
    if (!square || !square->bit()) {
        return nullptr;
    }
    return square->bit()->getOwner();
}

Player* Chess::checkForWinner()
{
    return nullptr;
}

bool Chess::checkForDraw()
{
    return false;
}

std::string Chess::initialStateString()
{
    return stateString();
}

std::string Chess::stateString()
{
    std::string s;
    s.reserve(64);
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
            s += pieceNotation( x, y );
        }
    );
    return s;}

void Chess::setStateString(const std::string &s)
{
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
        int index = y * 8 + x;
        char playerNumber = s[index] - '0';
        if (playerNumber) {
           square->setBit(PieceForPlayer(playerNumber - 1, Pawn));
        } else {
            square->setBit(nullptr);
        }
    });
}

