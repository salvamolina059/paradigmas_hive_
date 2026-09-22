#pragma once

#include <memory>

#include "hive/Enums.h"
#include "hive/movement/MovementStrategy.h"

/**
 * @brief `PieceType` -> `std::unique_ptr<MovementStrategy>`: la fábrica que dice
 *        qué estrategia de movimiento le corresponde a cada tipo.
 *
 * Es el único lugar del programa que sabe qué clase concreta va con cada tipo.
 * La llama el constructor de `Piece`, y la vuelve a llamar `Game` para resolver
 * a qué tipo eligió imitar el mosquito en un turno dado.
 *
 * Dos casos no son un tipo nuevo:
 * - `PieceType::Pillbug` devuelve una `QueenMovement`: el bicho bolita se mueve
 *   igual que la reina, así que no existe ninguna clase `PillbugMovement`.
 * - `PieceType::Mosquito` devuelve `nullptr`. No es un pendiente: es el
 *   comportamiento definitivo y correcto, porque no existe `MosquitoMovement`.
 *   Un mosquito recién construido no tiene movimiento propio hasta que `Game`
 *   le asigna uno —con esta misma fábrica, llamada de nuevo con el tipo que
 *   eligió imitar—; si está rodeado solo de mosquitos, `imitatableTypes()` viene
 *   vacío y nunca se le asigna nada. Ver Piece.h.
 *
 * @return La estrategia correspondiente, o `nullptr` para el mosquito.
 */
std::unique_ptr<MovementStrategy> createMovementStrategy(PieceType type);
