#pragma once

#include <vector>

#include "hive/Board.h"
#include "hive/Hex.h"
#include "hive/Piece.h"

// Esto no es una MovementStrategy: es una sola regla reusable con dos
// disparadores distintos (un bicho bolita de verdad, y un mosquito parado al
// lado de exactamente un bicho bolita, que le presta la habilidad en vez de
// reimplementarla). No hay varias implementaciones detrás de una interfaz común,
// así que una función libre encaja mejor que una jerarquía de clases.

// `pillbug` es el casillero de la pieza que usa la habilidad: un bicho bolita de
// verdad, o un mosquito que se la presta. pillbugThrows() no lo necesita —quien
// la llama ya tiene `at`—, pero hace falta para que un PillbugThrow sea una
// acción autosuficiente cuando se usa como una de las variantes de Move.
struct PillbugThrow {
    Hex pillbug;
    Hex victim;
    Hex destination;
};

inline bool operator==(const PillbugThrow &a, const PillbugThrow &b) {
    return a.pillbug == b.pillbug && a.victim == b.victim && a.destination == b.destination;
}

// Todos los pares (víctima, destino) que puede ejecutar este turno una pieza con
// la habilidad del bicho bolita, parada en `at`.
//
//   - víctima: un vecino ocupado de `at`, sola en su casillero
//     (stackHeight == 1, una pila nunca se puede lanzar), no clavada
//     (board.canMove(victimHex)) y distinta de `lastMoved` (puede ser nullptr:
//     la pieza que se movió el turno pasado, si hay, no se puede lanzar).
//   - destino: un vecino vacío de `at`, o sea pegado al que lanza, no
//     necesariamente al casillero donde estaba la víctima.
//
// No se chequea canSlide (la víctima se levanta y se apoya, no se desliza) ni el
// contacto con la colmena en el destino: queda garantizado, porque es vecino de
// `at`, que sin dudas es parte de la colmena.
//
// Lo que no chequea es si el que lanza es `lastMoved`: eso lo filtra Game antes,
// igual que chequea una sola vez si la pieza está clavada antes de consultar
// cualquier MovementStrategy, en vez de volver a deducirlo acá.
std::vector<PillbugThrow> pillbugThrows(const Board &board, const Hex &at,
                                         const Piece *lastMoved);
