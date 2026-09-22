#pragma once

#include "hive/movement/MovementStrategy.h"

/**
 * @brief Escarabajo: un paso a cualquier vecino, vacío u ocupado (se sube).
 *
 * Es la única pieza que puede terminar arriba de otra. Un escarabajo que está
 * subiendo, o que ya está parado sobre una pila, pasa por encima de las piezas
 * de al lado en vez de meterse entre ellas: las reglas del piso —deslizarse por
 * un hueco, seguir tocando la colmena— no le aplican igual.
 *
 * @note Que la altura importe es propio del escarabajo: `canSlide()` se queda a
 *       nivel del piso, porque la reina, la araña y la hormiga nunca se apilan.
 */
class BeetleMovement : public MovementStrategy {
public:
    /** @copydoc MovementStrategy::moves */
    std::vector<Hex> moves(const Board &board, const Hex &from,
                            const Piece &self) const override;
};
