#include <gtest/gtest.h>
#include "hive/Board.h"
#include "hive/Hex.h"
#include "hive/Piece.h"
#include "hive/movement/SpiderMovement.h"

#include "support/PrintBoard.h"

#include <algorithm>
#include <vector>

// La geometría que comparten todos los tests de abajo: una "pared" derecha de 5
// piezas en (0,0)..(4,0), con la araña estacionada en (0,-1), justo arriba de la punta
// izquierda de la pared, en la fila del perímetro de arriba. Ese perímetro
// —(0,-1),(1,-1),(2,-1),(3,-1),(4,-1)— es también una línea derecha, y cada casillero
// toca una o dos piezas de la pared, así que la araña tiene una caminata limpia de 3
// pasos: (0,-1) -> (1,-1) -> (2,-1) -> (3,-1).
//
// Los casilleros que flanquean cada paso (calculados, no sacados a ojo): paso 1
// (0,-1)->(1,-1) flanquean (0,0) [pared] y (1,-2) [vacío]; paso 2 (1,-1)->(2,-1)
// flanquean (1,0) [pared] y (2,-2) [vacío]; paso 3 (2,-1)->(3,-1) flanquean (2,0)
// [pared] y (3,-2) [vacío].

namespace {

bool Contains(const std::vector<Hex>& hexes, const Hex& target) {
    return std::find(hexes.begin(), hexes.end(), target) != hexes.end();
}

struct WallFixture {
    Board board;
    Piece spider{Color::White, PieceType::Spider};
    Piece w0{Color::Black, PieceType::Queen};
    Piece w1{Color::Black, PieceType::Queen};
    Piece w2{Color::Black, PieceType::Queen};
    Piece w3{Color::Black, PieceType::Queen};
    Piece w4{Color::Black, PieceType::Queen};

    WallFixture() {
        board.place(&spider, Hex{0, -1});
        board.place(&w0, Hex{0, 0});
        board.place(&w1, Hex{1, 0});
        board.place(&w2, Hex{2, 0});
        board.place(&w3, Hex{3, 0});
        board.place(&w4, Hex{4, 0});
    }
};

}  // namespace

// --- distancia: exactamente 3 pasos, ni uno menos -----------------------

TEST(SpiderMovement, ReachesExactlyDistanceThreeAlongAClearWall) {
    WallFixture fixture;
    ASSERT_EQ(distance(Hex{0, -1}, Hex{3, -1}), 3);

    SpiderMovement movement;
    const auto moves = movement.moves(fixture.board, Hex{0, -1}, fixture.spider);
    SCOPED_TRACE(Describe(fixture.board) + Describe(moves, "moves"));

    EXPECT_TRUE(Contains(moves, Hex{3, -1}));
    // Las paradas intermedias de la misma caminata no son destinos legales: la araña
    // mueve exactamente 3, no 1 ni 2.
    EXPECT_FALSE(Contains(moves, Hex{1, -1}));
    EXPECT_FALSE(Contains(moves, Hex{2, -1}));
}

// --- no vuelve sobre sus pasos -----------------------------------------

TEST(SpiderMovement, CannotReachAHexOnlyAvailableByRevisitingAStartingHex) {
    // El único vecino que le importa a (-1,-1) es (0,-1): el casillero desde donde
    // arranca la araña. La única forma de "llegar" en 3 pasos sería alejarse y volver
    // a pisar (0,-1) antes de seguir, o sea volver sobre sus pasos. No hay ningún
    // camino legal.
    WallFixture fixture;

    SpiderMovement movement;
    const auto moves = movement.moves(fixture.board, Hex{0, -1}, fixture.spider);

    SCOPED_TRACE(Describe(fixture.board) + Describe(moves, "moves"));

    EXPECT_FALSE(Contains(moves, Hex{-1, -1}));
}

// --- canSlide: el hueco se chequea en cada uno de los 3 pasos -----------

TEST(SpiderMovement, CannotSlideThroughAGateBlockedAtTheFirstStep) {
    WallFixture fixture;
    Piece blocker{Color::Black, PieceType::Ant};
    fixture.board.place(&blocker, Hex{1, -2});  // closes step 1's gate

    SpiderMovement movement;
    const auto moves = movement.moves(fixture.board, Hex{0, -1}, fixture.spider);

    SCOPED_TRACE(Describe(fixture.board) + Describe(moves, "moves"));

    EXPECT_FALSE(Contains(moves, Hex{3, -1}));
}

TEST(SpiderMovement, CannotSlideThroughAGateBlockedAtTheSecondStep) {
    WallFixture fixture;
    Piece blocker{Color::Black, PieceType::Ant};
    fixture.board.place(&blocker, Hex{2, -2});  // closes step 2's gate

    SpiderMovement movement;
    const auto moves = movement.moves(fixture.board, Hex{0, -1}, fixture.spider);

    SCOPED_TRACE(Describe(fixture.board) + Describe(moves, "moves"));

    EXPECT_FALSE(Contains(moves, Hex{3, -1}));
}

TEST(SpiderMovement, CannotSlideThroughAGateBlockedAtTheThirdStep) {
    WallFixture fixture;
    Piece blocker{Color::Black, PieceType::Ant};
    fixture.board.place(&blocker, Hex{3, -2});  // closes step 3's gate

    SpiderMovement movement;
    const auto moves = movement.moves(fixture.board, Hex{0, -1}, fixture.spider);

    SCOPED_TRACE(Describe(fixture.board) + Describe(moves, "moves"));

    EXPECT_FALSE(Contains(moves, Hex{3, -1}));
}

TEST(SpiderMovement, UnrelatedGateBlocksDoNotAffectAClearPath) {
    // Control de los 3 tests de arriba: tapar un hueco que NO está en la caminata
    // (0,-1)->(3,-1) deja ese camino legal.
    WallFixture fixture;
    Piece blocker{Color::Black, PieceType::Ant};
    fixture.board.place(&blocker, Hex{4, -2});  // not on this path

    SpiderMovement movement;
    const auto moves = movement.moves(fixture.board, Hex{0, -1}, fixture.spider);

    SCOPED_TRACE(Describe(fixture.board) + Describe(moves, "moves"));

    EXPECT_TRUE(Contains(moves, Hex{3, -1}));
}

// --- ramificación: más de 2 candidatos para el primer paso ---------------

TEST(SpiderMovement, ExploresBranchesFromTwoSeparatePointsOfContact) {
    // A diferencia de todos los fixtures de arriba, donde la araña toca la colmena en
    // un solo punto y eso da siempre exactamente 2 candidatos para el primer paso, una
    // araña en el MEDIO de una pared la toca en dos puntos separados —izquierda y
    // derecha— y tiene 4 candidatos (2 por lado), no 2. Una implementación que
    // "prueba 2 direcciones" exploraría un solo lado y se perdería el otro sin avisar.
    //
    // Pared: (-3,0),(-2,0),(-1,0), ARAÑA en (0,0), (1,0),(2,0),(3,0). Verificado
    // aparte (calculado, no a ojo) que tanto la caminata hacia la derecha hasta (3,-1)
    // como la de la izquierda hasta (-3,1) son caminos de 3 pasos completamente
    // válidos, y que la araña tiene los 4 candidatos para el primer paso: (1,-1),
    // (0,-1), (-1,1) y (0,1), dos flanqueando cada lado.
    Board board;
    Piece spider{Color::White, PieceType::Spider};
    Piece wl1{Color::Black, PieceType::Queen};
    Piece wl2{Color::Black, PieceType::Queen};
    Piece wl3{Color::Black, PieceType::Queen};
    Piece wr1{Color::Black, PieceType::Queen};
    Piece wr2{Color::Black, PieceType::Queen};
    Piece wr3{Color::Black, PieceType::Queen};
    board.place(&spider, Hex{0, 0});
    board.place(&wl1, Hex{-1, 0});
    board.place(&wl2, Hex{-2, 0});
    board.place(&wl3, Hex{-3, 0});
    board.place(&wr1, Hex{1, 0});
    board.place(&wr2, Hex{2, 0});
    board.place(&wr3, Hex{3, 0});

    SpiderMovement movement;
    const auto moves = movement.moves(board, Hex{0, 0}, spider);

    SCOPED_TRACE(Describe(board) + Describe(moves, "moves"));

    EXPECT_TRUE(Contains(moves, Hex{3, -1}))
        << "rightward walk (hugging the right-side attachment point)";
    EXPECT_TRUE(Contains(moves, Hex{-3, 1}))
        << "leftward walk (hugging the left-side attachment point)";
}

TEST(SpiderMovement, FindsDestinationsThatAreMerelyStepoversOnAnotherBranch) {
    // ExploresBranchesFromTwoSeparatePointsOfContact chequea un solo destino por lado,
    // y a ese un DFS con visitados GLOBAL —que nunca desmarca— igual lo encuentra. Lo
    // que pierde una implementación así es cada casillero por el que *pasó* en una rama
    // anterior: una vez marcado, una rama posterior no puede caer ahí, aunque
    // `visited` tenga que ser del camino (ver SpiderMovement.h).
    //
    // El mismo fixture del medio de la pared: pared (-3,0)..(3,0) y araña en (0,0).
    // (2,-1) es uno de esos casilleros: es la parada del paso 2 en la caminata a
    // (3,-1), y también un aterrizaje legítimo del paso 3 por (0,-1) -> (1,-1) ->
    // (2,-1). Sus tres espejos (2,1), (-2,-1) y (-2,1) son el mismo caso; el conjunto
    // completo de destinos es exactamente esos 8 casilleros.
    Board board;
    Piece spider{Color::White, PieceType::Spider};
    Piece wl1{Color::Black, PieceType::Queen};
    Piece wl2{Color::Black, PieceType::Queen};
    Piece wl3{Color::Black, PieceType::Queen};
    Piece wr1{Color::Black, PieceType::Queen};
    Piece wr2{Color::Black, PieceType::Queen};
    Piece wr3{Color::Black, PieceType::Queen};
    board.place(&spider, Hex{0, 0});
    board.place(&wl1, Hex{-1, 0});
    board.place(&wl2, Hex{-2, 0});
    board.place(&wl3, Hex{-3, 0});
    board.place(&wr1, Hex{1, 0});
    board.place(&wr2, Hex{2, 0});
    board.place(&wr3, Hex{3, 0});

    SpiderMovement movement;
    const auto moves = movement.moves(board, Hex{0, 0}, spider);

    // Solo se llega volviendo a entrar a un casillero por el que pasó una rama
    // anterior.
    SCOPED_TRACE(Describe(board) + Describe(moves, "moves"));

    EXPECT_TRUE(Contains(moves, Hex{2, -1}));
    EXPECT_TRUE(Contains(moves, Hex{2, 1}));
    EXPECT_TRUE(Contains(moves, Hex{-2, -1}));
    EXPECT_TRUE(Contains(moves, Hex{-2, 1}));
    // Las puntas de las cuatro caminatas de 3 pasos por las dos caras de la pared.
    EXPECT_TRUE(Contains(moves, Hex{3, -1}));
    EXPECT_TRUE(Contains(moves, Hex{-3, 1}));
    EXPECT_TRUE(Contains(moves, Hex{-1, -1}));
    EXPECT_TRUE(Contains(moves, Hex{1, 1}));
    EXPECT_EQ(moves.size(), 8u) << "no destination other than those 8";
}
