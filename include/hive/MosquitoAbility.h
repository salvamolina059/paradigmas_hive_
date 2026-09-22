#pragma once

#include <vector>

#include "hive/Board.h"
#include "hive/Enums.h"
#include "hive/Hex.h"

// Los tipos distintos que el mosquito podría imitar desde `at`: uno por cada
// PieceType distinto que aparezca entre los vecinos ocupados de `at`, mirando
// siempre la pieza de ARRIBA de cada pila vecina (topAt()). Que el vecino sea
// una pila no cambia nada: lo único que cuenta es qué pieza está operativa ahí.
// PieceType::Mosquito nunca se incluye: un mosquito no puede imitar a otro
// mosquito.
//
// Se usa para dos cosas distintas: juntar los movimientos de todos los tipos
// imitables (los MosquitoMovement, cuando el mosquito no está en altura) y
// decidir si le toca prestarse el lanzamiento del bicho bolita (cuando hay
// exactamente un tipo imitable y ese tipo es el bicho bolita).
std::vector<PieceType> imitatableTypes(const Board &board, const Hex &at);
