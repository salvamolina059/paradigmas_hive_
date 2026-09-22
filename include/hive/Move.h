#pragma once

#include <variant>

#include "hive/Enums.h"
#include "hive/Hex.h"
#include "hive/PillbugAbility.h"

// Las clases de acción que puede ser un turno. Son datos y nada más: quien
// realmente modifica el tablero y la mano del jugador es Game::applyMove(),
// según cuál de las variantes le toque.

// Colocar una pieza de `type`, sacada de la mano, en el casillero vacío
// `destination`.
struct Placement {
    PieceType type;
    Hex destination;
};

inline bool operator==(const Placement &a, const Placement &b) {
    return a.type == b.type && a.destination == b.destination;
}

// Mover una pieza ya colocada de `from` a `destination`, deslizándose o
// subiéndose.
struct Movement {
    Hex from;
    Hex destination;
};

inline bool operator==(const Movement &a, const Movement &b) {
    return a.from == b.from && a.destination == b.destination;
}

// PillbugThrow está definido en PillbugAbility.h (pillbug, victim, destination)
// y se reusa tal cual: ya era una acción completa y autosuficiente, no algo
// propio de la función que además lo devuelve.

// El mosquito, a nivel del piso, moviéndose como si imitara el tipo de una
// pieza vecina. `imitating` es un dato explícito y guardado —igual que
// PillbugThrow junta varios campos en una sola acción atómica—, no una elección
// que consuma un turno aparte: elegir y moverse pasan juntos, en una sola
// llamada a applyMove(), en un mismo turno.
//
// `imitating` puede ser PieceType::Pillbug: el movimiento propio del bicho
// bolita es QueenMovement (ver StrategyFactory), así que un mosquito que toca un
// bicho bolita recibe una entrada MosquitoMovement{imitating=Pillbug} igual que
// con cualquier otro tipo vecino. Eso es ADEMÁS de, y no en lugar de, los
// PillbugThrow que también recibe por tocarlo (le presta la habilidad), igual
// que Game::addMoves() le ofrece a un bicho bolita de verdad su Movement y su
// PillbugThrow al mismo tiempo.
//
// Nunca se genera mientras el mosquito está arriba de una pila: ahí la movida es
// un Movement común con el movimiento del escarabajo, sin elección de a quién
// imitar —llegó ahí imitando al escarabajo para subirse, y sigue siendo un
// escarabajo hasta que baja. Ver Game::legalMoves().
struct MosquitoMovement {
    Hex from;
    PieceType imitating;
    Hex destination;
};

inline bool operator==(const MosquitoMovement &a, const MosquitoMovement &b) {
    return a.from == b.from && a.imitating == b.imitating && a.destination == b.destination;
}

using Move = std::variant<Placement, Movement, PillbugThrow, MosquitoMovement>;
