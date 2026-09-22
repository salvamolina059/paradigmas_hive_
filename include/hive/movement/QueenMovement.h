#pragma once

#include "hive/movement/MovementStrategy.h"

/**
 * @brief Reina: un paso a un casillero vecino vacío.
 *
 * Es la regla más simple del juego, y la única pieza que se mueve "un casillero
 * y listo". Las dos reglas generales del tablero le aplican tal cual: no puede
 * pasar por un hueco tapado de los dos lados, y donde cae tiene que seguir
 * tocando la colmena. Las dos están implementadas en `Board`.
 *
 * @note Una reina sola en el tablero no tiene a qué quedar pegada, así que
 *       puede ir a cualquiera de sus 6 vecinos.
 */
class QueenMovement : public MovementStrategy {
public:
    /** @copydoc MovementStrategy::moves */
    std::vector<Hex> moves(const Board &board, const Hex &from,
                            const Piece &self) const override;
};
