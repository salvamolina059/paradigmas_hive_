#include "hive/movement/GrasshopperMovement.h"

#include "hive/Board.h"
#include "hive/Hex.h"

std::vector<Hex> GrasshopperMovement::moves(const Board &board, const Hex &from,
                                             const Piece &self) const
{
    (void)self;  // el saltamontes no lo necesita

    std::vector<Hex> destinos;

    for (const Hex &dir : hexDirections()) {
        Hex actual{from.q + dir.q, from.r + dir.r};

        // tiene que saltar por encima de al menos una pieza
        if (!board.isOccupied(actual)) {
            continue;
        }

        // avanza en linea recta hasta el primer casillero vacio
        while (board.isOccupied(actual)) {
            actual.q += dir.q;
            actual.r += dir.r;
        }

        destinos.push_back(actual);
    }

    return destinos;
}