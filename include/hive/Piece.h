#pragma once

#include <memory>
#include <vector>

#include "hive/Enums.h"
#include "hive/movement/MovementStrategy.h"

/**
 * @brief Una pieza del juego: su color, su tipo, y la estrategia de
 *        movimiento que le corresponde.
 *
 * La estrategia se engancha sola en el constructor, a través de
 * `createMovementStrategy()`. Una vez construida, la pieza nunca sabe de qué
 * tipo concreto es la estrategia que tiene: le delega y listo.
 */
struct Piece {
    Color color;
    PieceType type;
    std::unique_ptr<MovementStrategy> movement;

    /**
     * @brief Construye una pieza de `color` y `type`, con su estrategia de
     *        movimiento ya enganchada.
     *
     * Le pide la estrategia a `createMovementStrategy(type)`, y es dueña de lo
     * que recibe (`unique_ptr`): cuando la pieza se destruye, se destruye la
     * estrategia. Para `PieceType::Mosquito` queda en `nullptr` a propósito
     * (ver el comentario de la clase).
     */
    Piece(Color color, PieceType type);

    /**
     * @brief Todos los destinos a los que esta pieza puede ir desde `from`.
     *
     * No decide nada por su cuenta: le delega a su estrategia de movimiento.
     * Acá se ve el patrón entero en una línea —la pieza no pregunta de qué
     * tipo es.
     *
     * @param board El tablero, para consultar qué hay alrededor.
     * @param from  El casillero donde está la pieza ahora.
     * @return Los destinos legales, o una lista vacía si `movement` es
     *         `nullptr` o no hay movimiento posible (el caso del mosquito).
     */
    std::vector<Hex> legalMoves(const Board &board, const Hex &from) const;
};
