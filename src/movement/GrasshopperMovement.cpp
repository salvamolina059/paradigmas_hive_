#include "hive/movement/GrasshopperMovement.h"

#include "hive/Board.h"

// TODO (parte 2): implementen el movimiento del saltamontes (Grasshopper).
//
// La regla está en include/hive/movement/GrasshopperMovement.h, que además
// explica qué reglas generales NO se le aplican y por qué. Los tests están en
// tests/grasshopper_movement_test.cpp.
//
// hexDirections() (en Hex.h) da las 6 direcciones: saltar en línea recta es
// avanzar repetidamente sumando siempre la misma dirección.

std::vector<Hex> GrasshopperMovement::moves(const Board &board, const Hex &from,
                                             const Piece &self) const
{
    // TODO: devolver los destinos legales del saltamontes desde `from`.
    (void)board;
    (void)from;
    (void)self;
    return {};
}
