#include "hive/Piece.h"

#include "hive/movement/StrategyFactory.h"

Piece::Piece(Color color, PieceType type)
    : color(color), type(type), movement(createMovementStrategy(type))
{
}

std::vector<Hex> Piece::legalMoves(const Board &board, const Hex &from) const
{
    if (!movement)
    {
        return {};
    }
    return movement->moves(board, from, *this);
}
