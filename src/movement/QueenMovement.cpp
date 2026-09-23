#include "hive/movement/QueenMovement.h"

#include "hive/Board.h"

std::vector<Hex> QueenMovement::moves(const Board &board, const Hex &from,
                                      const Piece &self) const
{
    // la reina da un solo paso a un vecino vacio
    std::vector<Hex> destinos;

    for (const Hex &vecino : board.emptyNeighbors(from)) {
        // tiene que poder deslizarse y seguir pegada a la colmena
        if (board.canSlide(from, vecino) && board.wouldStayAttached(vecino, self)) {
            destinos.push_back(vecino);
        }
    }

    return destinos;
}