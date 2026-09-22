#include <gtest/gtest.h>
#include "hive/Board.h"
#include "hive/Hex.h"
#include "hive/Piece.h"
#include "hive/movement/GrasshopperMovement.h"

#include "support/PrintBoard.h"

#include <algorithm>
#include <vector>

namespace {

bool Contains(const std::vector<Hex>& hexes, const Hex& target) {
    return std::find(hexes.begin(), hexes.end(), target) != hexes.end();
}

}  // namespace

TEST(GrasshopperMovement, JumpsOverTwoConsecutivePiecesToTheFirstEmptyHex) {
    Board board;
    Piece hopper{Color::White, PieceType::Grasshopper};
    Piece p1{Color::Black, PieceType::Ant};
    Piece p2{Color::Black, PieceType::Spider};
    board.place(&hopper, Hex{0, 0});
    board.place(&p1, Hex{1, 0});
    board.place(&p2, Hex{2, 0});

    GrasshopperMovement movement;
    const auto moves = movement.moves(board, Hex{0, 0}, hopper);

    SCOPED_TRACE(Describe(board) + Describe(moves, "moves"));

    ASSERT_EQ(moves.size(), 1u);
    EXPECT_TRUE(Contains(moves, Hex{3, 0}));
}

TEST(GrasshopperMovement, EmptyImmediateNeighborBlocksTheWholeDirection) {
    // Hay una pieza más lejos, en (2,0), pero (1,0) —el vecino inmediato— está vacío,
    // así que no hay nada pegado para saltar. Toda esa dirección no sirve, no importa
    // qué haya más allá del hueco.
    Board board;
    Piece hopper{Color::White, PieceType::Grasshopper};
    Piece distant{Color::Black, PieceType::Ant};
    board.place(&hopper, Hex{0, 0});
    board.place(&distant, Hex{2, 0});

    GrasshopperMovement movement;
    const auto moves = movement.moves(board, Hex{0, 0}, hopper);

    SCOPED_TRACE(Describe(board) + Describe(moves, "moves"));

    EXPECT_TRUE(moves.empty());
}

TEST(GrasshopperMovement, IgnoresTheFreedomToMoveGate) {
    // Los dos casilleros que flanquean el lado (0,0)->(1,0) —(1,-1) y (0,1)— están
    // ocupados, lo que trabaría a una pieza que se desliza (ver
    // QueenMovement.CannotSlideThroughAGateBlockedOnBothSides y los tests de
    // canSlide). El saltamontes salta, así que la regla del hueco no le aplica.
    Board board;
    Piece hopper{Color::White, PieceType::Grasshopper};
    Piece jumped{Color::Black, PieceType::Ant};
    Piece flankA{Color::Black, PieceType::Spider};
    Piece flankB{Color::Black, PieceType::Beetle};
    board.place(&hopper, Hex{0, 0});
    board.place(&jumped, Hex{1, 0});
    board.place(&flankA, Hex{1, -1});
    board.place(&flankB, Hex{0, 1});

    GrasshopperMovement movement;
    const auto moves = movement.moves(board, Hex{0, 0}, hopper);

    SCOPED_TRACE(Describe(board) + Describe(moves, "moves"));

    EXPECT_TRUE(Contains(moves, Hex{2, 0}));
}

TEST(GrasshopperMovement, ExploresEachOfTheSixDirectionsIndependently) {
    // Dos líneas de salto activas, en direcciones opuestas; las otras 4 tienen el
    // vecino inmediato vacío y no aportan nada.
    Board board;
    Piece hopper{Color::White, PieceType::Grasshopper};
    Piece forward{Color::Black, PieceType::Ant};
    Piece backward{Color::Black, PieceType::Spider};
    board.place(&hopper, Hex{0, 0});
    board.place(&forward, Hex{1, 0});
    board.place(&backward, Hex{-1, 0});

    GrasshopperMovement movement;
    const auto moves = movement.moves(board, Hex{0, 0}, hopper);

    SCOPED_TRACE(Describe(board) + Describe(moves, "moves"));

    ASSERT_EQ(moves.size(), 2u);
    EXPECT_TRUE(Contains(moves, Hex{2, 0}));
    EXPECT_TRUE(Contains(moves, Hex{-2, 0}));
}
