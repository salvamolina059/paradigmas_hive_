#pragma once

#include "hive/movement/MovementStrategy.h"

/**
 * @brief Hormiga: todos los pasos que quiera, deslizándose por el borde de la
 *        colmena, y para donde llegue.
 *
 * A diferencia de la araña, esto es un solo BFS/DFS con un conjunto `visited`
 * **global**, sin volver atrás por cada camino: una vez que se sabe
 * que a un casillero se llega, volver a pasar por otro camino no descubre nada
 * nuevo. Y la hormiga puede parar en cualquier punto del recorrido, así que
 * todos los casilleros visitados menos `from` son destinos legales, no solo los
 * del final.
 *
 * Cada paso, del casillero actual `cur` a un candidato `next`, pide lo mismo
 * que cualquier pieza que se desliza:
 * - `next` está vacío y todavía no se visitó.
 * - `board.canSlide(cur, next)`.
 * - `board.wouldStayAttached(next, self)`.
 *
 * Como la cantidad de pasos no tiene tope, un hueco tapado en un camino casi
 * nunca alcanza para que un casillero sea inalcanzable: la hormiga da la vuelta
 * por otro lado. Quedan afuera solo los casilleros sin ninguna entrada legal
 * —por ejemplo el hueco en el medio de un anillo de piezas, donde todos los
 * vecinos están ocupados o son inalcanzables.
 */
class AntMovement : public MovementStrategy {
public:
    /** @copydoc MovementStrategy::moves */
    std::vector<Hex> moves(const Board &board, const Hex &from,
                            const Piece &self) const override;
};
