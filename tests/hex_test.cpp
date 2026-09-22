#include <gtest/gtest.h>
#include "hive/Hex.h"

#include <algorithm>
#include <vector>

namespace
{

    bool Contains(const std::vector<Hex> &hexes, const Hex &target)
    {
        return std::find(hexes.begin(), hexes.end(), target) != hexes.end();
    }

} // namespace

// --- neighbors() ------------------------------------------------------

TEST(HexNeighbors, OriginHasSixNeighbors)
{
    EXPECT_EQ(neighbors(Hex{0, 0}).size(), 6u);
}

TEST(HexNeighbors, OriginNeighborsAreTheSixAxialDirections)
{
    const std::vector<Hex> expected = {
        Hex{1, 0},
        Hex{1, -1},
        Hex{0, -1},
        Hex{-1, 0},
        Hex{-1, 1},
        Hex{0, 1},
    };
    const auto actual = neighbors(Hex{0, 0});

    for (const auto &e : expected)
    {
        EXPECT_TRUE(Contains(actual, e))
            << "missing neighbor (" << e.q << ", " << e.r << ")";
    }
}

TEST(HexNeighbors, NeighborsHaveNoDuplicates)
{
    const auto actual = neighbors(Hex{0, 0});
    for (size_t i = 0; i < actual.size(); ++i)
    {
        for (size_t j = i + 1; j < actual.size(); ++j)
        {
            EXPECT_FALSE(actual[i] == actual[j])
                << "duplicate neighbor at indices " << i << " and " << j;
        }
    }
}

TEST(HexNeighbors, NeighborOffsetsAreConsistentAwayFromOrigin)
{
    // Los vecinos de cualquier casillero tienen que ser los del origen, corridos
    // por ese casillero: las seis direcciones no cambian según dónde estés en el
    // tablero.
    const Hex center{2, -3};
    const auto originNeighbors = neighbors(Hex{0, 0});
    const auto centerNeighbors = neighbors(center);

    ASSERT_EQ(originNeighbors.size(), centerNeighbors.size());
    for (const auto &n : originNeighbors)
    {
        Hex expected{n.q + center.q, n.r + center.r};
        EXPECT_TRUE(Contains(centerNeighbors, expected))
            << "missing translated neighbor (" << expected.q << ", "
            << expected.r << ")";
    }
}

// --- distance() ---------------------------------------------------------

TEST(HexDistance, DistanceToSelfIsZero)
{
    EXPECT_EQ(distance(Hex{0, 0}, Hex{0, 0}), 0);
    EXPECT_EQ(distance(Hex{5, -2}, Hex{5, -2}), 0);
}

TEST(HexDistance, DistanceToEachImmediateNeighborIsOne)
{
    const Hex origin{0, 0};
    for (const auto &n : neighbors(origin))
    {
        EXPECT_EQ(distance(origin, n), 1)
            << "neighbor (" << n.q << ", " << n.r << ") should be distance 1";
    }
}

TEST(HexDistance, DistanceIsSymmetric)
{
    const Hex a{3, -1};
    const Hex b{-2, 4};
    EXPECT_EQ(distance(a, b), distance(b, a));
}

TEST(HexDistance, DistanceAlongAStraightLine)
{
    // Tres pasos en la misma dirección (+1, 0) desde el origen.
    EXPECT_EQ(distance(Hex{0, 0}, Hex{3, 0}), 3);
}

TEST(HexDistance, DistanceOnAMixedPath)
{
    // (0,0) -> (1,0) -> (2,0) -> (2,1): 3 pasos, y no hay camino más corto.
    EXPECT_EQ(distance(Hex{0, 0}, Hex{2, 1}), 3);
}
