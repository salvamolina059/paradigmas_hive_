#include "hive/movement/QueenMovement.h"

#include "hive/Board.h"

// TODO (parte 1b): implementen el movimiento de la reina (Queen).
//
// Es la primera estrategia del TP y la regla más simple de todas: un paso a un
// casillero vecino vacío. La regla está en
// include/hive/movement/QueenMovement.h; la consigna la explica con más
// detalle. AntMovement.cpp y SpiderMovement.cpp están resueltas y muestran la
// misma clase con un movimiento de varios pasos adentro. Los tests están en
// tests/queen_movement_test.cpp.
//
// Métodos de Board que van a necesitar: emptyNeighbors(), canSlide() y
// wouldStayAttached().

std::vector<Hex> QueenMovement::moves(const Board &board, const Hex &from,
                                      const Piece &self) const
{
    // TODO: devolver los destinos legales de la reina desde `from`.
    (void)board;
    (void)from;
    (void)self;
    return {};
}
