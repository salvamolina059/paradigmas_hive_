#include <gtest/gtest.h>
#include "hive/Board.h"
#include "hive/Hex.h"
#include "hive/Piece.h"
#include "hive/movement/QueenMovement.h"

#include "support/PrintBoard.h"

#include <algorithm>
#include <vector>

namespace {

bool Contains(const std::vector<Hex>& hexes, const Hex& target) {
    return std::find(hexes.begin(), hexes.end(), target) != hexes.end();
}

}  // namespace

TEST(Piece, ConstructorWiresTheRightMovementStrategy) {
    Piece queen{Color::White, PieceType::Queen};
    ASSERT_NE(queen.movement, nullptr);
    EXPECT_NE(dynamic_cast<QueenMovement*>(queen.movement.get()), nullptr);
}

TEST(Piece, MosquitoHasNoMovementStrategyAtConstruction) {
    Piece mosquito{Color::White, PieceType::Mosquito};
    EXPECT_EQ(mosquito.movement, nullptr);
}

TEST(Piece, LegalMovesDelegatesToTheAssignedStrategy) {
    Board board;
    Piece queen{Color::White, PieceType::Queen};
    board.place(&queen, Hex{0, 0});

    const auto viaPiece = queen.legalMoves(board, Hex{0, 0});
    const auto viaDirectStrategy = QueenMovement().moves(board, Hex{0, 0}, queen);

    SCOPED_TRACE(Describe(board) + Describe(viaPiece, "viaPiece")
                 + Describe(viaDirectStrategy, "viaDirectStrategy"));

    // Dos listas vacías son iguales, así que este test podría pasar sin probar la
    // delegación: justo el estado del esqueleto, donde QueenMovement es de la parte 1b
    // y la estrategia de Piece sale de la fábrica de la parte 2. Una reina sola en el
    // tablero tiene seis destinos, así que primero fijamos eso.
    ASSERT_FALSE(viaDirectStrategy.empty())
        << "QueenMovement returned nothing for a queen alone on the board";
    ASSERT_EQ(viaPiece.size(), viaDirectStrategy.size());
    for (const auto& h : viaDirectStrategy) {
        EXPECT_TRUE(Contains(viaPiece, h));
    }
}

TEST(Piece, LegalMovesIsEmptyRatherThanCrashingWhenMovementIsNull) {
    Board board;
    Piece mosquito{Color::White, PieceType::Mosquito};
    board.place(&mosquito, Hex{0, 0});

    const auto moves = mosquito.legalMoves(board, Hex{0, 0});

    SCOPED_TRACE(Describe(board) + Describe(moves, "moves"));

    EXPECT_TRUE(moves.empty());
}
