#include <gtest/gtest.h>
#include "hive/Move.h"

TEST(Move, HoldsAPlacement) {
    Move move = Placement{PieceType::Queen, Hex{0, 0}};

    ASSERT_TRUE(std::holds_alternative<Placement>(move));
    const auto& placement = std::get<Placement>(move);
    EXPECT_EQ(placement.type, PieceType::Queen);
    EXPECT_EQ(placement.destination.q, 0);
    EXPECT_EQ(placement.destination.r, 0);

    EXPECT_FALSE(std::holds_alternative<Movement>(move));
    EXPECT_FALSE(std::holds_alternative<PillbugThrow>(move));
}

TEST(Move, HoldsAMovement) {
    Move move = Movement{Hex{0, 0}, Hex{1, 0}};

    ASSERT_TRUE(std::holds_alternative<Movement>(move));
    const auto& movement = std::get<Movement>(move);
    EXPECT_EQ(movement.from.q, 0);
    EXPECT_EQ(movement.from.r, 0);
    EXPECT_EQ(movement.destination.q, 1);
    EXPECT_EQ(movement.destination.r, 0);
}

TEST(Move, HoldsAPillbugThrow) {
    Move move = PillbugThrow{Hex{0, 0}, Hex{1, 0}, Hex{-1, 0}};

    ASSERT_TRUE(std::holds_alternative<PillbugThrow>(move));
    const auto& t = std::get<PillbugThrow>(move);
    EXPECT_EQ(t.pillbug.q, 0);
    EXPECT_EQ(t.victim.q, 1);
    EXPECT_EQ(t.destination.q, -1);
}

TEST(Move, VisitDispatchesToTheCorrectKind) {
    Move move = Movement{Hex{0, 0}, Hex{1, 0}};

    int kindSeen = -1;
    std::visit(
        [&](auto&& m) {
            using T = std::decay_t<decltype(m)>;
            if constexpr (std::is_same_v<T, Placement>) {
                kindSeen = 0;
            } else if constexpr (std::is_same_v<T, Movement>) {
                kindSeen = 1;
            } else if constexpr (std::is_same_v<T, PillbugThrow>) {
                kindSeen = 2;
            }
        },
        move);

    EXPECT_EQ(kindSeen, 1);
}

// --- operator== --------------------------------------------------------
//
// No es un detalle académico: Game::applyMove() valida una movida buscándola en
// legalMoves() con std::find, así que la igualdad ES toda esa validación. Un campo que
// quede afuera de un operator== es un campo que applyMove() deja de chequear; en
// game_test.cpp, los dos tests ApplyMoveRejects... muestran qué se cuela por ahí.

TEST(Move, PlacementEqualityDistinguishesTheType) {
    const Placement queen{PieceType::Queen, Hex{0, 0}};

    EXPECT_TRUE(queen == (Placement{PieceType::Queen, Hex{0, 0}}));
    EXPECT_FALSE(queen == (Placement{PieceType::Ant, Hex{0, 0}}));
    EXPECT_FALSE(queen == (Placement{PieceType::Queen, Hex{1, 0}}));
}

TEST(Move, PillbugThrowEqualityDistinguishesEveryField) {
    const PillbugThrow t{Hex{0, 0}, Hex{1, 0}, Hex{-1, 0}};

    EXPECT_TRUE(t == (PillbugThrow{Hex{0, 0}, Hex{1, 0}, Hex{-1, 0}}));
    EXPECT_FALSE(t == (PillbugThrow{Hex{0, 0}, Hex{1, -1}, Hex{-1, 0}}));  // victim
    EXPECT_FALSE(t == (PillbugThrow{Hex{0, 0}, Hex{1, 0}, Hex{-1, 1}}));   // destination
    EXPECT_FALSE(t == (PillbugThrow{Hex{0, 1}, Hex{1, 0}, Hex{-1, 0}}));   // thrower
}

TEST(Move, MosquitoMovementEqualityDistinguishesTheImitatedType) {
    const MosquitoMovement m{Hex{0, 0}, PieceType::Beetle, Hex{1, 0}};

    EXPECT_TRUE(m == (MosquitoMovement{Hex{0, 0}, PieceType::Beetle, Hex{1, 0}}));
    EXPECT_FALSE(m == (MosquitoMovement{Hex{0, 0}, PieceType::Queen, Hex{1, 0}}));
    EXPECT_FALSE(m == (MosquitoMovement{Hex{0, 0}, PieceType::Beetle, Hex{1, -1}}));
}
