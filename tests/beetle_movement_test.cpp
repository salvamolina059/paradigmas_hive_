#include <gtest/gtest.h>
#include "hive/Board.h"
#include "hive/Hex.h"
#include "hive/Piece.h"
#include "hive/movement/BeetleMovement.h"

#include "support/PrintBoard.h"

#include <algorithm>
#include <vector>

namespace {

bool Contains(const std::vector<Hex>& hexes, const Hex& target) {
    return std::find(hexes.begin(), hexes.end(), target) != hexes.end();
}

}  // namespace

TEST(BeetleMovement, CanClimbOntoAnOccupiedNeighbor) {
    Board board;
    Piece beetle{Color::White, PieceType::Beetle};
    Piece other{Color::Black, PieceType::Queen};
    board.place(&beetle, Hex{0, 0});
    board.place(&other, Hex{1, 0});

    BeetleMovement movement;
    const auto moves = movement.moves(board, Hex{0, 0}, beetle);

    SCOPED_TRACE(Describe(board) + Describe(moves, "moves"));

    EXPECT_TRUE(Contains(moves, Hex{1, 0}));
}

TEST(BeetleMovement, HasQueensEmptyDestinationsPlusTheClimb) {
    // El mismo armado que QueenMovement.CannotMoveOntoAnOccupiedNeighborOrLoseContact:
    // con una sola otra pieza en (1,0), una pieza que camina por el piso solo llega,
    // entre los destinos vacíos, a los dos casilleros que flanquean ese lado: (1,-1) y
    // (0,1). El escarabajo tiene esos dos MÁS la subida a (1,0), que la reina no
    // puede hacer.
    Board board;
    Piece beetle{Color::White, PieceType::Beetle};
    Piece other{Color::Black, PieceType::Queen};
    board.place(&beetle, Hex{0, 0});
    board.place(&other, Hex{1, 0});

    BeetleMovement movement;
    const auto moves = movement.moves(board, Hex{0, 0}, beetle);

    SCOPED_TRACE(Describe(board) + Describe(moves, "moves"));

    ASSERT_EQ(moves.size(), 3u);
    EXPECT_TRUE(Contains(moves, Hex{1, -1}));
    EXPECT_TRUE(Contains(moves, Hex{0, 1}));
    EXPECT_TRUE(Contains(moves, Hex{1, 0}));
}

TEST(BeetleMovement, GroundLevelBeetleIsStillBlockedByAGateToAnEmptyHex) {
    // Control: sin altura de por medio, el hueco le funciona al escarabajo igual que a
    // la reina. Con los dos casilleros que flanquean (0,0)->(1,0) ocupados, no se
    // puede deslizar al (1,0) vacío.
    Board board;
    Piece beetle{Color::White, PieceType::Beetle};
    Piece blockerA{Color::Black, PieceType::Ant};
    Piece blockerB{Color::Black, PieceType::Spider};
    board.place(&beetle, Hex{0, 0});
    board.place(&blockerA, Hex{1, -1});
    board.place(&blockerB, Hex{0, 1});

    BeetleMovement movement;
    const auto moves = movement.moves(board, Hex{0, 0}, beetle);

    SCOPED_TRACE(Describe(board) + Describe(moves, "moves"));

    EXPECT_FALSE(Contains(moves, Hex{1, 0}));
}

TEST(BeetleMovement, ElevatedBeetleBypassesAGateThatWouldBlockAGroundPiece) {
    // El escarabajo está arriba de una pila de dos en (0,0): se subió a una pieza que
    // ya estaba. Los dos casilleros que flanquean el lado (0,0)->(1,0) están
    // ocupados, lo que trabaría a una pieza que camina por el piso (ver el test de
    // arriba), pero un escarabajo en altura les pasa por encima en vez de meterse
    // entre ellas.
    Board board;
    Piece base{Color::White, PieceType::Queen};
    Piece beetle{Color::White, PieceType::Beetle};
    Piece blockerA{Color::Black, PieceType::Ant};
    Piece blockerB{Color::Black, PieceType::Spider};
    board.place(&base, Hex{0, 0});
    board.place(&beetle, Hex{0, 0});  // beetle climbs on top, stackHeight == 2
    board.place(&blockerA, Hex{1, -1});
    board.place(&blockerB, Hex{0, 1});

    BeetleMovement movement;
    const auto moves = movement.moves(board, Hex{0, 0}, beetle);

    SCOPED_TRACE(Describe(board) + Describe(moves, "moves"));

    EXPECT_TRUE(Contains(moves, Hex{1, 0}));
}
