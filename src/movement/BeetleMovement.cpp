#include "hive/movement/BeetleMovement.h"

#include "hive/Board.h"

// TODO (parte 2): implementen el movimiento del escarabajo (Beetle).
//
// La regla está en include/hive/movement/BeetleMovement.h; la consigna la
// explica con más detalle. La reina de la parte 1b es la misma forma con una
// regla más simple, y AntMovement.cpp está resuelta. Los tests están en
// tests/beetle_movement_test.cpp.
//
// Métodos de Board que van a necesitar: neighbors() (en Hex.h), isOccupied(),
// stackHeight(), canSlide() y wouldStayAttached().

std::vector<Hex> BeetleMovement::moves(const Board &board, const Hex &from,
                                        const Piece &self) const
{
    // TODO: devolver los destinos legales del escarabajo desde `from`.
    (void)board;
    (void)from;
    (void)self;
    return {};
}
