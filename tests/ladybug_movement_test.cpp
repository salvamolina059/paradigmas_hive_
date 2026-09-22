#include <gtest/gtest.h>
#include "hive/Board.h"
#include "hive/Hex.h"
#include "hive/Piece.h"
#include "hive/movement/LadybugMovement.h"

#include "support/PrintBoard.h"

#include <algorithm>
#include <vector>

namespace {

bool Contains(const std::vector<Hex>& hexes, const Hex& target) {
    return std::find(hexes.begin(), hexes.end(), target) != hexes.end();
}

}  // namespace

TEST(LadybugMovement, ClimbsTwoOccupiedHexesAndDescendsToAnEmptyHex) {
    Board board;
    Piece ladybug{Color::White, PieceType::Ladybug};
    Piece p1{Color::Black, PieceType::Ant};
    Piece p2{Color::Black, PieceType::Spider};
    board.place(&ladybug, Hex{0, 0});
    board.place(&p1, Hex{1, 0});
    board.place(&p2, Hex{2, 0});

    LadybugMovement movement;
    const auto moves = movement.moves(board, Hex{0, 0}, ladybug);

    SCOPED_TRACE(Describe(board) + Describe(moves, "moves"));

    EXPECT_TRUE(Contains(moves, Hex{3, 0}));
    // Nunca para arriba de la colmena: siempre baja a un casillero vacío.
    EXPECT_FALSE(Contains(moves, Hex{1, 0}));
    EXPECT_FALSE(Contains(moves, Hex{2, 0}));
}

TEST(LadybugMovement, IncludesDestinationsFoundOnlyViaATwoHopDetour) {
    // A y B son vecinos DIRECTOS (a 1 salto) de la vaquita, y además vecinos entre sí.
    // from->B->A es una subida de 2 pasos legítima, sin volver sobre sus pasos, que
    // termina en A, aunque a A también se llegue en un solo salto. Así que A —y por
    // simetría B— igual cuentan como posición válida "a 2 pasos". Una implementación
    // ingenua que usa la distancia más corta de BFS == 2 no encontraría NINGUNA
    // posición a 2 saltos acá (A y B están a distancia más corta 1) y devolvería cero
    // movidas legales, mal.
    Board board;
    Piece ladybug{Color::White, PieceType::Ladybug};
    Piece a{Color::Black, PieceType::Ant};
    Piece b{Color::Black, PieceType::Spider};
    board.place(&ladybug, Hex{0, 0});
    board.place(&a, Hex{1, 0});
    board.place(&b, Hex{1, -1});

    LadybugMovement movement;
    const auto moves = movement.moves(board, Hex{0, 0}, ladybug);

    SCOPED_TRACE(Describe(board) + Describe(moves, "moves"));

    EXPECT_FALSE(moves.empty());
    EXPECT_TRUE(Contains(moves, Hex{2, 0}))    << "only reachable via A as a 2-hop position";
    EXPECT_TRUE(Contains(moves, Hex{0, -1}))   << "only reachable via B as a 2-hop position";
}

TEST(LadybugMovement, IgnoresTheFreedomToMoveGateThroughoutTheClimb) {
    // Los dos casilleros que flanquean el lado (0,0)->(1,0) —(1,-1) y (0,1)— están
    // ocupados, lo que le trabaría por completo el primer paso a una pieza que se
    // desliza (ver QueenMovement.CannotSlideThroughAGateBlockedOnBothSides). La
    // vaquita nunca chequea canSlide, así que sube por (1,0) hasta (2,0) y todavía
    // puede bajar a (3,0), más allá.
    Board board;
    Piece ladybug{Color::White, PieceType::Ladybug};
    Piece piece1{Color::Black, PieceType::Ant};
    Piece piece2{Color::Black, PieceType::Beetle};
    Piece flankA{Color::Black, PieceType::Spider};
    Piece flankB{Color::Black, PieceType::Grasshopper};
    board.place(&ladybug, Hex{0, 0});
    board.place(&piece1, Hex{1, 0});
    board.place(&piece2, Hex{2, 0});
    board.place(&flankA, Hex{1, -1});
    board.place(&flankB, Hex{0, 1});

    LadybugMovement movement;
    const auto moves = movement.moves(board, Hex{0, 0}, ladybug);

    SCOPED_TRACE(Describe(board) + Describe(moves, "moves"));

    EXPECT_TRUE(Contains(moves, Hex{3, 0}));
}

TEST(LadybugMovement, NeverUsesItsOwnHexAsOneOfTheTwoStepsAcrossTheHive) {
    // El conjunto de los casilleros a 2 saltos se arma sumando pasos y tiene que
    // excluir `from`: la vaquita ya se levantó del tablero, así que su propio casillero
    // no es algo por lo que pueda pasar. Cadena: vaquita (0,0) - (1,0) - (2,0).
    //
    // La única posición a 2 saltos es (2,0), así que todos los destinos legales son
    // vecinos vacíos de (2,0). Contar (0,0) también como posición a 2 saltos
    // (from -> (1,0) -> y volver) agregaría sus propios vecinos vacíos: casilleros como
    // (-1,0), atrás de la vaquita, a los que ninguna subida de 3 pasos llega.
    Board board;
    Piece ladybug{Color::White, PieceType::Ladybug};
    Piece p1{Color::Black, PieceType::Ant};
    Piece p2{Color::Black, PieceType::Spider};
    board.place(&ladybug, Hex{0, 0});
    board.place(&p1, Hex{1, 0});
    board.place(&p2, Hex{2, 0});

    LadybugMovement movement;
    const auto moves = movement.moves(board, Hex{0, 0}, ladybug);

    SCOPED_TRACE(Describe(board) + Describe(moves, "moves"));

    EXPECT_FALSE(Contains(moves, Hex{-1, 0}));
    EXPECT_FALSE(Contains(moves, Hex{-1, 1}));
    EXPECT_FALSE(Contains(moves, Hex{0, -1}));
    EXPECT_FALSE(Contains(moves, Hex{0, 1}));
    EXPECT_FALSE(Contains(moves, Hex{1, -1}));

    // Exactamente los vecinos vacíos de la única posición a 2 saltos, (2,0).
    EXPECT_EQ(moves.size(), 5u);
    EXPECT_TRUE(Contains(moves, Hex{3, 0}));
    EXPECT_TRUE(Contains(moves, Hex{3, -1}));
    EXPECT_TRUE(Contains(moves, Hex{2, -1}));
    EXPECT_TRUE(Contains(moves, Hex{2, 1}));
    EXPECT_TRUE(Contains(moves, Hex{1, 1}));
}
