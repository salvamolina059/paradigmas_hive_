#pragma once

#include <vector>

#include "hive/Hex.h"

class Board;
struct Piece;

/**
 * @brief La interfaz de "cómo se mueve este tipo de pieza".
 *
 * Cada tipo tiene su propia clase que la implementa —`QueenMovement`,
 * `SpiderMovement`, …— con su propia regla adentro. Una `Piece` es dueña de una
 * de estas (`unique_ptr<MovementStrategy>`) y le delega `legalMoves()` sin
 * saber nunca de qué tipo concreto es.
 *
 * `moves()` contesta una sola pregunta: dada la forma de moverse de esta pieza
 * —su recorrido, qué casilleros puede pisar, la regla del hueco—, a dónde puede
 * ir, suponiendo que se pueda mover. No se fija en:
 *
 * - La regla de la colmena del lado del casillero de partida
 *   (`Board::canMove(from)`): es idéntica para todas las piezas, así que `Game`
 *   la consulta una sola vez antes de preguntarle a cualquier estrategia, en
 *   vez de repetirla en cada una.
 * - Si `self` es la pieza de arriba de su pila: `Game` llama a esto únicamente
 *   para `board.topAt(from)`.
 */
class MovementStrategy {
public:
    virtual ~MovementStrategy() = default;

    /**
     * @brief Los destinos legales de esta pieza desde `from`.
     *
     * @param board El tablero, para consultar qué hay alrededor. No se
     *              modifica: la pieza se "levanta" preguntando, no moviéndola
     *              de verdad.
     * @param from  El casillero donde está la pieza ahora.
     * @param self  La pieza que se mueve. Hace falta para preguntar por la
     *              regla de la colmena sin contarse a sí misma, y para otras
     *              operaciones en las que se necesite reconocer su propio
     *              casillero.
     * @return Los casilleros a los que puede ir, en cualquier orden y sin
     *         repetidos.
     */
    virtual std::vector<Hex> moves(const Board& board, const Hex& from,
                                    const Piece& self) const = 0;
};
