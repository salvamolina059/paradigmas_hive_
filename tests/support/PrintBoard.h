#pragma once

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

#include "hive/Board.h"
#include "hive/Enums.h"
#include "hive/Hex.h"
#include "hive/Piece.h"

// Para imprimir el tablero y la lista de movidas en los mensajes de error de gtest.
//
// Un test de movimiento que falla imprime "esperaba true, obtuve false" y nada más,
// que no dice nada sobre con qué tablero pasó. Las coordenadas hexagonales son lo
// bastante difíciles de tener en la cabeza como para que imprimir el tablero sea, casi
// siempre, la diferencia entre ver el bug y adivinarlo.
//
// Hay dos formas de usarlo. Colgado de un assert puntual:
//
//   EXPECT_TRUE(Contains(moves, Hex{3, 0})) << Describe(board) << Describe(moves);
//
// o con SCOPED_TRACE, una vez por test, después de armar el tablero y calcular el
// resultado. Así vale para TODOS los asserts que vengan abajo, incluso los que se
// agreguen después:
//
//   SCOPED_TRACE(Describe(board) + Describe(moves, "moves"));
//
// Así están escritos los tests de corrección de tests/. El segundo parámetro de
// Describe es una etiqueta, y sirve sobre todo cuando un test mira dos listas a la
// vez y hay que saber cuál es cuál.

inline const char *PieceTypeName(PieceType type)
{
    switch (type)
    {
    case PieceType::Queen:       return "Queen";
    case PieceType::Spider:      return "Spider";
    case PieceType::Beetle:      return "Beetle";
    case PieceType::Grasshopper: return "Grasshopper";
    case PieceType::Ant:         return "Ant";
    case PieceType::Mosquito:    return "Mosquito";
    case PieceType::Ladybug:     return "Ladybug";
    case PieceType::Pillbug:     return "Pillbug";
    }
    return "?";
}

// Ordenado para que dos impresiones del mismo tablero se lean igual: occupiedHexes()
// sale de un unordered_map y su orden no es estable entre corridas.
inline std::vector<Hex> SortedHexes(std::vector<Hex> hexes)
{
    std::sort(hexes.begin(), hexes.end(), [](const Hex &a, const Hex &b) {
        return a.q != b.q ? a.q < b.q : a.r < b.r;
    });
    return hexes;
}

inline std::string Describe(const Board &board)
{
    std::ostringstream out;
    const std::vector<Hex> hexes = SortedHexes(board.occupiedHexes());
    out << "\n  board (" << hexes.size() << " occupied):";
    for (const Hex &h : hexes)
    {
        const Piece *top = board.topAt(h);
        out << "\n    (" << h.q << ", " << h.r << ")  "
            << (top->color == Color::White ? "white " : "black ") << PieceTypeName(top->type);
        if (board.stackHeight(h) > 1)
        {
            out << "  [stack of " << board.stackHeight(h) << "]";
        }
    }
    return out.str();
}

inline std::string Describe(const std::vector<Hex> &hexes, const char *label = "hexes")
{
    std::ostringstream out;
    out << "\n  " << label << " (" << hexes.size() << "): ";
    for (const Hex &h : SortedHexes(hexes))
    {
        out << "(" << h.q << ", " << h.r << ") ";
    }
    return out.str();
}
