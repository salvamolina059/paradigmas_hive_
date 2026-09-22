#include <gtest/gtest.h>
#include "hive/Board.h"
#include "hive/Hex.h"
#include "hive/Piece.h"

#include "support/PrintBoard.h"

#include <algorithm>
#include <vector>

// La API de Board que asume esta tanda de tests (colocación y ocupación nada más:
// isHiveConnected / canSlide / la legalidad de una colocación quedan afuera a
// propósito, porque se prueban de otra manera):
//
//   class Board {
//   public:
//       void place(Piece* piece, const Hex& at);
//       void move(const Hex& from, const Hex& to);
//       Piece* remove(const Hex& at);
//
//       Piece* topAt(const Hex& at) const;       // nullptr if empty
//       bool isOccupied(const Hex& at) const;
//       std::size_t stackHeight(const Hex& at) const;
//       std::vector<Hex> occupiedHexes() const;
//       std::vector<Hex> emptyNeighbors(const Hex& at) const;
//   };
//
// Board no es dueña de ninguna pieza (guarda Piece* sin propiedad, según el modelo
// de ownership: el dueño de verdad es Game). move() y place() apilan y desapilan sin
// preguntar: Board no se fija si alguien tiene derecho a caer ahí, eso lo decide
// quien llama.

namespace {

bool Contains(const std::vector<Hex>& hexes, const Hex& target) {
    return std::find(hexes.begin(), hexes.end(), target) != hexes.end();
}

}  // namespace

// --- tablero vacío -------------------------------------------------------

TEST(BoardOccupancy, NewBoardIsEmpty) {
    Board board;
    SCOPED_TRACE(Describe(board));

    EXPECT_FALSE(board.isOccupied(Hex{0, 0}));
    EXPECT_TRUE(board.occupiedHexes().empty());
    EXPECT_EQ(board.stackHeight(Hex{0, 0}), 0u);
    EXPECT_EQ(board.topAt(Hex{0, 0}), nullptr);
}

// --- place() --------------------------------------------------------------

TEST(BoardOccupancy, PlacingAPieceMakesItsHexOccupied) {
    Board board;
    Piece queen{Color::White, PieceType::Queen};
    board.place(&queen, Hex{0, 0});

    SCOPED_TRACE(Describe(board));

    EXPECT_TRUE(board.isOccupied(Hex{0, 0}));
    EXPECT_EQ(board.topAt(Hex{0, 0}), &queen);
    EXPECT_EQ(board.stackHeight(Hex{0, 0}), 1u);
}

TEST(BoardOccupancy, PiecesAtDifferentHexesAreIndependent) {
    Board board;
    Piece a{Color::White, PieceType::Ant};
    Piece b{Color::Black, PieceType::Spider};
    board.place(&a, Hex{0, 0});
    board.place(&b, Hex{1, 0});

    SCOPED_TRACE(Describe(board));

    EXPECT_EQ(board.topAt(Hex{0, 0}), &a);
    EXPECT_EQ(board.topAt(Hex{1, 0}), &b);
    EXPECT_FALSE(board.isOccupied(Hex{2, 0}));
}

TEST(BoardOccupancy, OccupiedHexesReturnsExactlyThePlacedHexes) {
    Board board;
    Piece a{Color::White, PieceType::Ant};
    Piece b{Color::Black, PieceType::Spider};
    Piece c{Color::White, PieceType::Grasshopper};
    board.place(&a, Hex{0, 0});
    board.place(&b, Hex{1, -1});
    board.place(&c, Hex{-1, 1});

    const auto occupied = board.occupiedHexes();
    SCOPED_TRACE(Describe(board) + Describe(occupied, "occupied"));

    ASSERT_EQ(occupied.size(), 3u);
    EXPECT_TRUE(Contains(occupied, Hex{0, 0}));
    EXPECT_TRUE(Contains(occupied, Hex{1, -1}));
    EXPECT_TRUE(Contains(occupied, Hex{-1, 1}));
}

// --- move(): pilas LIFO, como cuando sube un escarabajo o un mosquito -----

TEST(BoardStacking, MovingOntoAnOccupiedHexStacksOnTop) {
    Board board;
    Piece base{Color::White, PieceType::Queen};
    Piece climber{Color::Black, PieceType::Beetle};
    board.place(&base, Hex{0, 0});
    board.place(&climber, Hex{1, 0});

    board.move(Hex{1, 0}, Hex{0, 0});

    SCOPED_TRACE(Describe(board));

    EXPECT_EQ(board.stackHeight(Hex{0, 0}), 2u);
    EXPECT_EQ(board.topAt(Hex{0, 0}), &climber);
    EXPECT_FALSE(board.isOccupied(Hex{1, 0}));
    EXPECT_EQ(board.stackHeight(Hex{1, 0}), 0u);
}

TEST(BoardStacking, MovingTheTopOffAStackRevealsThePieceBelow) {
    Board board;
    Piece base{Color::White, PieceType::Queen};
    Piece climber{Color::Black, PieceType::Beetle};
    board.place(&base, Hex{0, 0});
    board.place(&climber, Hex{1, 0});
    board.move(Hex{1, 0}, Hex{0, 0});  // climber stacks on base

    board.move(Hex{0, 0}, Hex{2, 0});  // climber moves off again

    SCOPED_TRACE(Describe(board));

    EXPECT_EQ(board.stackHeight(Hex{0, 0}), 1u);
    EXPECT_EQ(board.topAt(Hex{0, 0}), &base);
    EXPECT_EQ(board.topAt(Hex{2, 0}), &climber);
    EXPECT_FALSE(board.isOccupied(Hex{1, 0}));
}

// --- remove() ---------------------------------------------------------

TEST(BoardOccupancy, RemoveTakesThePieceOffTheBoardAndReturnsIt) {
    Board board;
    Piece piece{Color::White, PieceType::Ladybug};
    board.place(&piece, Hex{0, 0});

    Piece* removed = board.remove(Hex{0, 0});

    SCOPED_TRACE(Describe(board));

    EXPECT_EQ(removed, &piece);
    EXPECT_FALSE(board.isOccupied(Hex{0, 0}));
    EXPECT_EQ(board.stackHeight(Hex{0, 0}), 0u);
}

// --- emptyNeighbors() ---------------------------------------------------

TEST(BoardEmptyNeighbors, AllSixAreEmptyOnAnEmptyBoard) {
    Board board;
    const auto empty = board.emptyNeighbors(Hex{0, 0});
    const auto expected = neighbors(Hex{0, 0});

    SCOPED_TRACE(Describe(board) + Describe(empty, "empty"));

    ASSERT_EQ(empty.size(), 6u);
    for (const auto& n : expected) {
        EXPECT_TRUE(Contains(empty, n));
    }
}

TEST(BoardEmptyNeighbors, ExcludesOccupiedNeighbors) {
    Board board;
    Piece piece{Color::Black, PieceType::Grasshopper};
    board.place(&piece, Hex{1, 0});

    const auto empty = board.emptyNeighbors(Hex{0, 0});

    SCOPED_TRACE(Describe(board) + Describe(empty, "empty"));

    EXPECT_EQ(empty.size(), 5u);
    EXPECT_FALSE(Contains(empty, Hex{1, 0}));
}

// --- occupiedNeighbors(): lo usan el mosquito y la vaquita ----------------

TEST(BoardOccupiedNeighbors, NoneAreOccupiedOnAnEmptyBoard) {
    Board board;
    SCOPED_TRACE(Describe(board));

    EXPECT_TRUE(board.occupiedNeighbors(Hex{0, 0}).empty());
}

TEST(BoardOccupiedNeighbors, IncludesOnlyOccupiedNeighbors) {
    Board board;
    Piece piece{Color::Black, PieceType::Grasshopper};
    board.place(&piece, Hex{1, 0});

    const auto occupied = board.occupiedNeighbors(Hex{0, 0});

    SCOPED_TRACE(Describe(board) + Describe(occupied, "occupied"));

    ASSERT_EQ(occupied.size(), 1u);
    EXPECT_TRUE(Contains(occupied, Hex{1, 0}));
}

TEST(BoardOccupiedNeighbors, EmptyAndOccupiedNeighborsPartitionAllSix) {
    // Cada vecino de `at` está en exactamente una de las dos listas,
    // emptyNeighbors() o occupiedNeighbors(): juntas tienen que dar siempre 6.
    Board board;
    Piece a{Color::White, PieceType::Ant};
    Piece b{Color::Black, PieceType::Beetle};
    board.place(&a, Hex{1, 0});
    board.place(&b, Hex{-1, 1});

    const auto empty = board.emptyNeighbors(Hex{0, 0});
    const auto occupied = board.occupiedNeighbors(Hex{0, 0});

    SCOPED_TRACE(Describe(board) + Describe(empty, "empty") + Describe(occupied, "occupied"));

    EXPECT_EQ(occupied.size(), 2u);
    EXPECT_EQ(empty.size() + occupied.size(), 6u);
    EXPECT_TRUE(Contains(occupied, Hex{1, 0}));
    EXPECT_TRUE(Contains(occupied, Hex{-1, 1}));
}

// --- isSurrounded(): lo usa Game para detectar victoria y empate -----------

TEST(BoardIsSurrounded, EmptyBoardOriginIsNotSurrounded) {
    Board board;
    SCOPED_TRACE(Describe(board));

    EXPECT_FALSE(board.isSurrounded(Hex{0, 0}));
}

TEST(BoardIsSurrounded, FiveOfSixNeighborsOccupiedIsNotSurrounded) {
    Board board;
    Piece pieces[5] = {
        Piece{Color::White, PieceType::Ant},  Piece{Color::White, PieceType::Ant},
        Piece{Color::White, PieceType::Ant},  Piece{Color::White, PieceType::Ant},
        Piece{Color::White, PieceType::Ant},
    };
    const auto ns = neighbors(Hex{0, 0});
    for (int i = 0; i < 5; ++i) {
        board.place(&pieces[i], ns[i]);
    }

    SCOPED_TRACE(Describe(board));

    EXPECT_FALSE(board.isSurrounded(Hex{0, 0}));
}

TEST(BoardIsSurrounded, AllSixNeighborsOccupiedIsSurrounded) {
    Board board;
    Piece pieces[6] = {
        Piece{Color::White, PieceType::Ant}, Piece{Color::Black, PieceType::Ant},
        Piece{Color::White, PieceType::Ant}, Piece{Color::Black, PieceType::Ant},
        Piece{Color::White, PieceType::Ant}, Piece{Color::Black, PieceType::Ant},
    };
    const auto ns = neighbors(Hex{0, 0});
    for (int i = 0; i < 6; ++i) {
        board.place(&pieces[i], ns[i]);
    }

    SCOPED_TRACE(Describe(board));

    EXPECT_TRUE(board.isSurrounded(Hex{0, 0}));
}

// --- contains(): lo usa Game para encontrar una reina aunque esté enterrada -

TEST(BoardContains, FalseOnAnEmptyHex) {
    Board board;
    SCOPED_TRACE(Describe(board));

    EXPECT_FALSE(board.contains(Hex{0, 0}, Color::White, PieceType::Queen));
}

TEST(BoardContains, TrueWhenTheMatchingPieceIsOnTop) {
    Board board;
    Piece queen{Color::White, PieceType::Queen};
    board.place(&queen, Hex{0, 0});

    SCOPED_TRACE(Describe(board));

    EXPECT_TRUE(board.contains(Hex{0, 0}, Color::White, PieceType::Queen));
    EXPECT_FALSE(board.contains(Hex{0, 0}, Color::Black, PieceType::Queen));
    EXPECT_FALSE(board.contains(Hex{0, 0}, Color::White, PieceType::Ant));
}

TEST(BoardContains, TrueWhenTheMatchingPieceIsBuriedUnderAnotherPiece) {
    Board board;
    Piece queen{Color::Black, PieceType::Queen};
    Piece beetle{Color::White, PieceType::Beetle};
    board.place(&queen, Hex{0, 0});
    board.place(&beetle, Hex{0, 0});  // stacks on top, burying the queen

    SCOPED_TRACE(Describe(board));

    EXPECT_EQ(board.topAt(Hex{0, 0}), &beetle);
    EXPECT_TRUE(board.contains(Hex{0, 0}, Color::Black, PieceType::Queen));
    EXPECT_TRUE(board.contains(Hex{0, 0}, Color::White, PieceType::Beetle));
}
