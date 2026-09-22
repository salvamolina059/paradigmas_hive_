#include <gtest/gtest.h>
#include "hive/Board.h"
#include "hive/Enums.h"
#include "hive/Hex.h"
#include "hive/MosquitoAbility.h"
#include "hive/Piece.h"

#include "support/PrintBoard.h"

#include <algorithm>
#include <vector>

namespace {

bool Contains(const std::vector<PieceType>& types, PieceType target) {
    return std::find(types.begin(), types.end(), target) != types.end();
}

}  // namespace

TEST(MosquitoAbility, EmptyWhenAlone) {
    Board board;
    Piece mosquito{Color::White, PieceType::Mosquito};
    board.place(&mosquito, Hex{0, 0});

    const auto types = imitatableTypes(board, Hex{0, 0});

    SCOPED_TRACE(Describe(board));

    EXPECT_TRUE(types.empty());
}

TEST(MosquitoAbility, ListsEachDistinctNeighboringType) {
    Board board;
    Piece mosquito{Color::White, PieceType::Mosquito};
    Piece queen{Color::Black, PieceType::Queen};
    Piece spider{Color::Black, PieceType::Spider};
    board.place(&mosquito, Hex{0, 0});
    board.place(&queen, Hex{1, 0});
    board.place(&spider, Hex{1, -1});

    const auto types = imitatableTypes(board, Hex{0, 0});

    SCOPED_TRACE(Describe(board));

    ASSERT_EQ(types.size(), 2u);
    EXPECT_TRUE(Contains(types, PieceType::Queen));
    EXPECT_TRUE(Contains(types, PieceType::Spider));
}

TEST(MosquitoAbility, DoesNotDuplicateTheSameTypeSeenTwice) {
    Board board;
    Piece mosquito{Color::White, PieceType::Mosquito};
    Piece queenA{Color::Black, PieceType::Queen};
    Piece queenB{Color::White, PieceType::Queen};
    board.place(&mosquito, Hex{0, 0});
    board.place(&queenA, Hex{1, 0});
    board.place(&queenB, Hex{1, -1});

    const auto types = imitatableTypes(board, Hex{0, 0});

    SCOPED_TRACE(Describe(board));

    ASSERT_EQ(types.size(), 1u);
    EXPECT_TRUE(Contains(types, PieceType::Queen));
}

TEST(MosquitoAbility, NeverIncludesMosquitoItself) {
    Board board;
    Piece mosquito{Color::White, PieceType::Mosquito};
    Piece otherMosquito{Color::Black, PieceType::Mosquito};
    Piece queen{Color::Black, PieceType::Queen};
    board.place(&mosquito, Hex{0, 0});
    board.place(&otherMosquito, Hex{1, 0});
    board.place(&queen, Hex{1, -1});

    const auto types = imitatableTypes(board, Hex{0, 0});

    SCOPED_TRACE(Describe(board));

    ASSERT_EQ(types.size(), 1u);
    EXPECT_TRUE(Contains(types, PieceType::Queen));
}

TEST(MosquitoAbility, NeighborsOwnStackDepthDoesNotMatter) {
    // El casillero vecino tiene una pila de dos: una reina abajo y un escarabajo
    // arriba. imitatableTypes tiene que reflejar al escarabajo (topAt()), sin que le
    // importe que el casillero sea una pila.
    Board board;
    Piece mosquito{Color::White, PieceType::Mosquito};
    Piece base{Color::Black, PieceType::Queen};
    Piece top{Color::Black, PieceType::Beetle};
    board.place(&mosquito, Hex{0, 0});
    board.place(&base, Hex{1, 0});
    board.place(&top, Hex{1, 0});  // stacks on top

    const auto types = imitatableTypes(board, Hex{0, 0});

    SCOPED_TRACE(Describe(board));

    ASSERT_EQ(types.size(), 1u);
    EXPECT_TRUE(Contains(types, PieceType::Beetle));
    EXPECT_FALSE(Contains(types, PieceType::Queen));
}
