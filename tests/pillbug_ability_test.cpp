#include <gtest/gtest.h>
#include "hive/Board.h"
#include "hive/Hex.h"
#include "hive/Piece.h"
#include "hive/PillbugAbility.h"

#include "support/PrintBoard.h"

#include <algorithm>
#include <vector>

namespace {

bool Contains(const std::vector<PillbugThrow>& throws, const Hex& victim,
              const Hex& destination) {
    return std::any_of(throws.begin(), throws.end(), [&](const PillbugThrow& t) {
        return t.victim.q == victim.q && t.victim.r == victim.r &&
               t.destination.q == destination.q && t.destination.r == destination.r;
    });
}

bool AnyWithVictim(const std::vector<PillbugThrow>& throws, const Hex& victim) {
    return std::any_of(throws.begin(), throws.end(), [&](const PillbugThrow& t) {
        return t.victim.q == victim.q && t.victim.r == victim.r;
    });
}

}  // namespace

TEST(PillbugAbility, CanThrowASingleAdjacentPieceToAnEmptyNeighbor) {
    Board board;
    Piece pillbug{Color::White, PieceType::Pillbug};
    Piece victim{Color::Black, PieceType::Ant};
    board.place(&pillbug, Hex{0, 0});
    board.place(&victim, Hex{1, 0});

    const auto throws = pillbugThrows(board, Hex{0, 0}, nullptr);

    SCOPED_TRACE(Describe(board));

    EXPECT_TRUE(Contains(throws, Hex{1, 0}, Hex{-1, 0}));
}

TEST(PillbugAbility, CannotThrowAStackedPiece) {
    Board board;
    Piece pillbug{Color::White, PieceType::Pillbug};
    Piece base{Color::Black, PieceType::Queen};
    Piece top{Color::Black, PieceType::Beetle};
    board.place(&pillbug, Hex{0, 0});
    board.place(&base, Hex{1, 0});
    board.place(&top, Hex{1, 0});  // stacks on top, stackHeight == 2

    const auto throws = pillbugThrows(board, Hex{0, 0}, nullptr);

    SCOPED_TRACE(Describe(board));

    EXPECT_FALSE(AnyWithVictim(throws, Hex{1, 0}));
}

TEST(PillbugAbility, CannotThrowAPinnedPiece) {
    // extra está conectada a la colmena solamente a través de victim: lanzar a victim
    // dejaría a extra separada, así que victim está clavada.
    Board board;
    Piece pillbug{Color::White, PieceType::Pillbug};
    Piece victim{Color::Black, PieceType::Ant};
    Piece extra{Color::Black, PieceType::Spider};
    board.place(&pillbug, Hex{0, 0});
    board.place(&victim, Hex{1, 0});
    board.place(&extra, Hex{2, 0});

    const auto throws = pillbugThrows(board, Hex{0, 0}, nullptr);

    SCOPED_TRACE(Describe(board));

    EXPECT_FALSE(AnyWithVictim(throws, Hex{1, 0}));
}

TEST(PillbugAbility, CannotThrowThePieceThatMovedLastTurn) {
    Board board;
    Piece pillbug{Color::White, PieceType::Pillbug};
    Piece victim{Color::Black, PieceType::Ant};
    board.place(&pillbug, Hex{0, 0});
    board.place(&victim, Hex{1, 0});

    const auto throws = pillbugThrows(board, Hex{0, 0}, &victim);

    SCOPED_TRACE(Describe(board));

    EXPECT_FALSE(AnyWithVictim(throws, Hex{1, 0}));
}

TEST(PillbugAbility, ProducesTheFullCrossProductOfVictimsAndDestinations) {
    // v1 y v2 son las dos víctimas válidas: son vecinas del bicho bolita y vecinas
    // entre sí, así que saquemos la que saquemos, el resto sigue conectado. Con solo v1
    // y v2 ocupados, los otros 4 vecinos del bicho bolita son destinos vacíos, así que
    // el producto completo es 2 víctimas x 4 destinos = 8 lanzamientos.
    Board board;
    Piece pillbug{Color::White, PieceType::Pillbug};
    Piece v1{Color::Black, PieceType::Ant};
    Piece v2{Color::Black, PieceType::Spider};
    board.place(&pillbug, Hex{0, 0});
    board.place(&v1, Hex{1, 0});
    board.place(&v2, Hex{1, -1});

    const auto throws = pillbugThrows(board, Hex{0, 0}, nullptr);

    const std::vector<Hex> destinations = {
        Hex{0, -1}, Hex{-1, 0}, Hex{-1, 1}, Hex{0, 1},
    };
    SCOPED_TRACE(Describe(board));

    ASSERT_EQ(throws.size(), 8u);
    for (const Hex& d : destinations) {
        EXPECT_TRUE(Contains(throws, Hex{1, 0}, d));
        EXPECT_TRUE(Contains(throws, Hex{1, -1}, d));
    }
}

TEST(PillbugAbility, NeverThrowsToAnOccupiedHex) {
    Board board;
    Piece pillbug{Color::White, PieceType::Pillbug};
    Piece victim{Color::Black, PieceType::Ant};
    Piece other{Color::Black, PieceType::Spider};
    board.place(&pillbug, Hex{0, 0});
    board.place(&victim, Hex{1, 0});
    board.place(&other, Hex{1, -1});

    const auto throws = pillbugThrows(board, Hex{0, 0}, nullptr);

    for (const auto& t : throws) {
        SCOPED_TRACE(Describe(board));

        EXPECT_FALSE(t.destination.q == 1 && t.destination.r == 0);
        EXPECT_FALSE(t.destination.q == 1 && t.destination.r == -1);
    }
}
