#pragma once

#include "hive/movement/MovementStrategy.h"

/**
 * @brief Araña: exactamente 3 pasos por el borde de la colmena, sin volver a
 *        pisar ningún casillero de este mismo recorrido (`from` incluido).
 *
 * Cada paso, del casillero actual `cur` a un candidato `next`, pide lo mismo
 * que el único paso de la reina:
 * - `next` está vacío y no es parte del camino que viene haciendo.
 * - `board.canSlide(cur, next)`.
 * - `board.wouldStayAttached(next, self)`.
 *
 * Los tres chequeos miran siempre el tablero original: lo único que se considera
 * levantado es `self`, el resto de la colmena no se mueve en medio del
 * recorrido. Por eso los pasos 1, 2 y 3 se chequean exactamente igual.
 */
class SpiderMovement : public MovementStrategy {
public:
    /** @copydoc MovementStrategy::moves */
    std::vector<Hex> moves(const Board &board, const Hex &from,
                            const Piece &self) const override;
};
