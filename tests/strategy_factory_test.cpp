#include <gtest/gtest.h>
#include "hive/Enums.h"
#include "hive/movement/AntMovement.h"
#include "hive/movement/BeetleMovement.h"
#include "hive/movement/GrasshopperMovement.h"
#include "hive/movement/LadybugMovement.h"
#include "hive/movement/QueenMovement.h"
#include "hive/movement/SpiderMovement.h"
#include "hive/movement/StrategyFactory.h"

TEST(StrategyFactory, CreatesQueenMovementForQueen) {
    auto strategy = createMovementStrategy(PieceType::Queen);
    ASSERT_NE(strategy, nullptr);
    EXPECT_NE(dynamic_cast<QueenMovement*>(strategy.get()), nullptr);
}

TEST(StrategyFactory, CreatesSpiderMovementForSpider) {
    auto strategy = createMovementStrategy(PieceType::Spider);
    ASSERT_NE(strategy, nullptr);
    EXPECT_NE(dynamic_cast<SpiderMovement*>(strategy.get()), nullptr);
}

TEST(StrategyFactory, CreatesBeetleMovementForBeetle) {
    auto strategy = createMovementStrategy(PieceType::Beetle);
    ASSERT_NE(strategy, nullptr);
    EXPECT_NE(dynamic_cast<BeetleMovement*>(strategy.get()), nullptr);
}

TEST(StrategyFactory, CreatesGrasshopperMovementForGrasshopper) {
    auto strategy = createMovementStrategy(PieceType::Grasshopper);
    ASSERT_NE(strategy, nullptr);
    EXPECT_NE(dynamic_cast<GrasshopperMovement*>(strategy.get()), nullptr);
}

TEST(StrategyFactory, CreatesAntMovementForAnt) {
    auto strategy = createMovementStrategy(PieceType::Ant);
    ASSERT_NE(strategy, nullptr);
    EXPECT_NE(dynamic_cast<AntMovement*>(strategy.get()), nullptr);
}

TEST(StrategyFactory, CreatesLadybugMovementForLadybug) {
    auto strategy = createMovementStrategy(PieceType::Ladybug);
    ASSERT_NE(strategy, nullptr);
    EXPECT_NE(dynamic_cast<LadybugMovement*>(strategy.get()), nullptr);
}

TEST(StrategyFactory, PillbugReusesQueenMovement) {
    // No existe ninguna clase PillbugMovement: el movimiento propio del bicho bolita es
    // idéntico al de la reina, así que la fábrica reparte una QueenMovement.
    auto strategy = createMovementStrategy(PieceType::Pillbug);
    ASSERT_NE(strategy, nullptr);
    EXPECT_NE(dynamic_cast<QueenMovement*>(strategy.get()), nullptr);
}

// PieceType::Mosquito no tiene test acá porque no hay nada que mapear: la fábrica
// devuelve nullptr a propósito, y quién le asigna una estrategia es Game, según a
// quién elija imitar ese turno (ver StrategyFactory.h).
