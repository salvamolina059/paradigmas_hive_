#include <gtest/gtest.h>
#include "hive/Board.h"
#include "hive/BoardGraphAdapter.h"
#include "hive/Hex.h"
#include "hive/Piece.h"

#include "support/PrintBoard.h"

#include <algorithm>
#include <vector>

namespace {

bool Contains(const std::vector<Hex>& hexes, const Hex& target) {
    return std::find(hexes.begin(), hexes.end(), target) != hexes.end();
}

}  // namespace

// --- isConnected() ------------------------------------------------------

TEST(BoardGraphAdapterConnectivity, EmptyBoardIsConnected) {
    Board board;
    BoardGraphAdapter graph(board);
    SCOPED_TRACE(Describe(board));

    EXPECT_TRUE(graph.isConnected());
}

TEST(BoardGraphAdapterConnectivity, SingleOccupiedHexIsConnected) {
    Board board;
    Piece piece{Color::White, PieceType::Queen};
    board.place(&piece, Hex{0, 0});

    BoardGraphAdapter graph(board);
    SCOPED_TRACE(Describe(board));

    EXPECT_TRUE(graph.isConnected());
}

TEST(BoardGraphAdapterConnectivity, TwoAdjacentHexesAreConnected) {
    Board board;
    Piece a{Color::White, PieceType::Queen};
    Piece b{Color::Black, PieceType::Ant};
    board.place(&a, Hex{0, 0});
    board.place(&b, Hex{1, 0});

    BoardGraphAdapter graph(board);
    SCOPED_TRACE(Describe(board));

    EXPECT_TRUE(graph.isConnected());
}

TEST(BoardGraphAdapterConnectivity, TwoDisconnectedHexesAreNotConnected) {
    Board board;
    Piece a{Color::White, PieceType::Queen};
    Piece b{Color::Black, PieceType::Ant};
    board.place(&a, Hex{0, 0});
    board.place(&b, Hex{5, 5});  // far away, no path

    BoardGraphAdapter graph(board);
    SCOPED_TRACE(Describe(board));

    EXPECT_FALSE(graph.isConnected());
}

TEST(BoardGraphAdapterConnectivity, ChainOfThreeIsConnected) {
    Board board;
    Piece a{Color::White, PieceType::Queen};
    Piece b{Color::Black, PieceType::Ant};
    Piece c{Color::White, PieceType::Spider};
    board.place(&a, Hex{0, 0});
    board.place(&b, Hex{1, 0});
    board.place(&c, Hex{2, 0});

    BoardGraphAdapter graph(board);
    SCOPED_TRACE(Describe(board));

    EXPECT_TRUE(graph.isConnected());
}

// --- ignoring: así se pregunta si una pieza está clavada -----------------

TEST(BoardGraphAdapterIgnoring, IgnoringACutHexDisconnectsTheRest) {
    // Una cadena derecha: (0,0) - (1,0) - (2,0). El casillero del medio es el único
    // vínculo entre las dos puntas.
    Board board;
    Piece a{Color::White, PieceType::Queen};
    Piece b{Color::Black, PieceType::Ant};
    Piece c{Color::White, PieceType::Spider};
    board.place(&a, Hex{0, 0});
    board.place(&b, Hex{1, 0});
    board.place(&c, Hex{2, 0});

    const Hex ignored{1, 0};
    BoardGraphAdapter graph(board, &ignored);

    SCOPED_TRACE(Describe(board));

    EXPECT_FALSE(graph.isConnected());
}

TEST(BoardGraphAdapterIgnoring, IgnoringANonCutHexKeepsTheRestConnected) {
    // (0,0), (1,0) y (1,-1) son vecinos entre sí (un "triángulo"), así que saquemos
    // cualquiera, los otros dos siguen siendo vecinos.
    Board board;
    Piece a{Color::White, PieceType::Queen};
    Piece b{Color::Black, PieceType::Ant};
    Piece c{Color::White, PieceType::Spider};
    board.place(&a, Hex{0, 0});
    board.place(&b, Hex{1, 0});
    board.place(&c, Hex{1, -1});

    const Hex ignored{1, 0};
    BoardGraphAdapter graph(board, &ignored);

    SCOPED_TRACE(Describe(board));

    EXPECT_TRUE(graph.isConnected());
}

TEST(BoardGraphAdapterIgnoring, IgnoredHexIsExcludedFromNeighbors) {
    Board board;
    Piece a{Color::White, PieceType::Queen};
    Piece b{Color::Black, PieceType::Ant};
    board.place(&a, Hex{0, 0});
    board.place(&b, Hex{1, 0});

    const Hex ignored{1, 0};
    BoardGraphAdapter graph(board, &ignored);

    SCOPED_TRACE(Describe(board));

    EXPECT_TRUE(graph.neighbors(Hex{0, 0}).empty());
}

// --- neighbors() ---------------------------------------------------------

TEST(BoardGraphAdapterNeighbors, ReturnsOnlyOccupiedAdjacentHexes) {
    Board board;
    Piece a{Color::White, PieceType::Queen};
    Piece b{Color::Black, PieceType::Ant};
    board.place(&a, Hex{0, 0});
    board.place(&b, Hex{1, 0});

    BoardGraphAdapter graph(board);
    const auto ns = graph.neighbors(Hex{0, 0});

    SCOPED_TRACE(Describe(board) + Describe(ns, "ns"));

    ASSERT_EQ(ns.size(), 1u);
    EXPECT_TRUE(Contains(ns, Hex{1, 0}));
}

TEST(BoardGraphAdapterNeighbors, HexNotInGraphHasNoNeighbors) {
    Board board;
    Piece a{Color::White, PieceType::Queen};
    board.place(&a, Hex{0, 0});

    BoardGraphAdapter graph(board);
    SCOPED_TRACE(Describe(board));

    EXPECT_TRUE(graph.neighbors(Hex{9, 9}).empty());
}

// --- distancesFrom() -------------------------------------------------

TEST(BoardGraphAdapterDistances, DistancesAlongAChain) {
    Board board;
    Piece a{Color::White, PieceType::Queen};
    Piece b{Color::Black, PieceType::Ant};
    Piece c{Color::White, PieceType::Spider};
    board.place(&a, Hex{0, 0});
    board.place(&b, Hex{1, 0});
    board.place(&c, Hex{2, 0});

    BoardGraphAdapter graph(board);
    const auto distances = graph.distancesFrom(Hex{0, 0});

    SCOPED_TRACE(Describe(board));

    ASSERT_EQ(distances.size(), 3u);
    EXPECT_EQ(distances.at(Hex{0, 0}), 0);
    EXPECT_EQ(distances.at(Hex{1, 0}), 1);
    EXPECT_EQ(distances.at(Hex{2, 0}), 2);
}

TEST(BoardGraphAdapterDistances, UnreachableNodeHasDistanceMinusOne) {
    Board board;
    Piece a{Color::White, PieceType::Queen};
    Piece b{Color::Black, PieceType::Ant};
    board.place(&a, Hex{0, 0});
    board.place(&b, Hex{5, 5});

    BoardGraphAdapter graph(board);
    const auto distances = graph.distancesFrom(Hex{0, 0});

    SCOPED_TRACE(Describe(board));

    ASSERT_EQ(distances.count(Hex{5, 5}), 1u) << "unreachable nodes are still in the map";
    EXPECT_EQ(distances.at(Hex{5, 5}), -1);
}

TEST(BoardGraphAdapterDistances, SourceNotInGraphReturnsEmptyMap) {
    Board board;
    Piece a{Color::White, PieceType::Queen};
    board.place(&a, Hex{0, 0});

    BoardGraphAdapter graph(board);
    SCOPED_TRACE(Describe(board));

    EXPECT_TRUE(graph.distancesFrom(Hex{9, 9}).empty());
}
