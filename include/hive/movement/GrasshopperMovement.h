#pragma once

#include "hive/movement/MovementStrategy.h"

/**
 * @brief Saltamontes: en cada una de las 6 direcciones, si el vecino está
 *        ocupado, salta en línea recta por encima de todas las piezas seguidas
 *        y cae en el primer casillero vacío.
 *
 * Si el vecino en esa dirección está vacío no hay nada que saltar, y esa
 * dirección no da ningún movimiento, no importa lo que haya más lejos.
 *
 * A diferencia de las piezas que se deslizan, el saltamontes ignora:
 *
 * - `canSlide()`, la regla del hueco: salta por encima de la colmena en vez de
 *   deslizarse por al lado, así que lo de quedar apretado entre dos piezas no
 *   le aplica.
 * - `wouldStayAttached()`: no hace falta chequearlo, queda garantizado por cómo
 *   salta. El casillero donde cae es siempre vecino de la última pieza que
 *   saltó —es justamente el primero *vacío*—, así que nunca puede caer suelto.
 * - La altura (`stackHeight`): salta por encima de casilleros ocupados sin
 *   importar cuán alta sea la pila.
 */
class GrasshopperMovement : public MovementStrategy {
public:
    /** @copydoc MovementStrategy::moves */
    std::vector<Hex> moves(const Board &board, const Hex &from,
                            const Piece &self) const override;
};
