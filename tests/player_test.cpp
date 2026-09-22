#include <gtest/gtest.h>
#include "hive/Player.h"

TEST(Player, StartsWithBasePlusExpansionsCounts) {
    Player player{Color::White};

    EXPECT_EQ(player.remaining(PieceType::Queen), 1);
    EXPECT_EQ(player.remaining(PieceType::Spider), 2);
    EXPECT_EQ(player.remaining(PieceType::Beetle), 2);
    EXPECT_EQ(player.remaining(PieceType::Grasshopper), 3);
    EXPECT_EQ(player.remaining(PieceType::Ant), 3);
    EXPECT_EQ(player.remaining(PieceType::Mosquito), 1);
    EXPECT_EQ(player.remaining(PieceType::Ladybug), 1);
    EXPECT_EQ(player.remaining(PieceType::Pillbug), 1);
}

TEST(Player, ColorMatchesConstruction) {
    Player white{Color::White};
    Player black{Color::Black};

    EXPECT_EQ(white.color(), Color::White);
    EXPECT_EQ(black.color(), Color::Black);
}

TEST(Player, PlaceDecrementsOnlyThatType) {
    Player player{Color::White};

    player.place(PieceType::Spider);

    EXPECT_EQ(player.remaining(PieceType::Spider), 1);
    EXPECT_EQ(player.remaining(PieceType::Beetle), 2);  // unaffected
}

TEST(Player, PlaceDecrementsEachTimeItsCalled) {
    Player player{Color::White};

    player.place(PieceType::Ant);
    player.place(PieceType::Ant);

    EXPECT_EQ(player.remaining(PieceType::Ant), 1);
}

TEST(Player, HasPlacedQueenIsFalseUntilTheQueenIsPlaced) {
    Player player{Color::White};

    EXPECT_FALSE(player.hasPlacedQueen());

    player.place(PieceType::Queen);

    EXPECT_TRUE(player.hasPlacedQueen());
}
