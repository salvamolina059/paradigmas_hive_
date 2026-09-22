#include <gtest/gtest.h>
#include "HexLayout.h"

#include "hive/Hex.h"

#include <cmath>

// --- hexToPixel / pixelToHex: ida y vuelta -----------------------------

TEST(HexLayout, RoundTripsEveryHexInRange)
{
    const HexLayout layout(40.f, {400.f, 300.f});
    for (int q = -6; q <= 6; ++q)
    {
        for (int r = -6; r <= 6; ++r)
        {
            const Hex h{q, r};
            const Hex roundTripped = layout.pixelToHex(layout.hexToPixel(h));
            EXPECT_TRUE(roundTripped == h)
                << "expected (" << q << ", " << r << ") but got ("
                << roundTripped.q << ", " << roundTripped.r << ")";
        }
    }
}

TEST(HexLayout, SizeReturnsWhatItWasConstructedWith)
{
    const HexLayout layout(27.5f, {0.f, 0.f});
    EXPECT_FLOAT_EQ(layout.size(), 27.5f);
}

TEST(HexLayout, OriginPixelMapsToOriginHex)
{
    const HexLayout layout(40.f, {400.f, 300.f});
    const Hex h = layout.pixelToHex({400.f, 300.f});
    EXPECT_TRUE(h == (Hex{0, 0})) << "got (" << h.q << ", " << h.r << ")";
}

TEST(HexLayout, PointNudgedOffCenterStillResolvesToSameHex)
{
    const HexLayout layout(40.f, {0.f, 0.f});
    const Hex expected{2, -1};
    const sf::Vector2f center = layout.hexToPixel(expected);
    // Unos pocos píxeles corridos del centro siguen estando bien adentro de un
    // hexágono de 40px.
    const sf::Vector2f nudged{center.x + 5.f, center.y - 3.f};

    const Hex actual = layout.pixelToHex(nudged);
    EXPECT_TRUE(actual == expected)
        << "expected (" << expected.q << ", " << expected.r << ") but got ("
        << actual.q << ", " << actual.r << ")";
}

TEST(HexLayout, EachNeighborDirectionRoundTrips)
{
    const HexLayout layout(40.f, {0.f, 0.f});
    const Hex origin{0, 0};
    for (const Hex &dir : hexDirections())
    {
        const Hex expected{origin.q + dir.q, origin.r + dir.r};
        const Hex actual = layout.pixelToHex(layout.hexToPixel(expected));
        EXPECT_TRUE(actual == expected)
            << "expected (" << expected.q << ", " << expected.r << ") but got ("
            << actual.q << ", " << actual.r << ")";
    }
}

// --- geometría absoluta (la ida y vuelta sola no la fija) ---------------

TEST(HexLayout, HexToPixelUsesFlatTopAxialGeometry)
{
    // hexToPixel y pixelToHex coinciden entre sí con CUALQUIER separación de
    // columnas —son inversas por construcción—, así que todos los tests de ida y
    // vuelta de arriba siguen pasando aunque las constantes se corran. Lo único que
    // fija el dibujo que hace la GUI son coordenadas de pantalla concretas.
    const float size = 40.f;
    const HexLayout layout(size, {0.f, 0.f});
    const float sqrt3 = std::sqrt(3.f);

    EXPECT_NEAR(layout.hexToPixel(Hex{1, 0}).x, 1.5f * size, 1e-3f);
    EXPECT_NEAR(layout.hexToPixel(Hex{1, 0}).y, sqrt3 * 0.5f * size, 1e-3f);
    EXPECT_NEAR(layout.hexToPixel(Hex{0, 1}).x, 0.f, 1e-3f);
    EXPECT_NEAR(layout.hexToPixel(Hex{0, 1}).y, sqrt3 * size, 1e-3f);
    EXPECT_NEAR(layout.hexToPixel(Hex{2, 0}).x, 3.f * size, 1e-3f);
}

TEST(HexLayout, EveryNeighborCenterIsExactlyOneHexWidthAway)
{
    // Los hexágonos vecinos tienen que dibujarse lado contra lado: para un hexágono
    // de circunradio `size`, eso significa centros separados sqrt(3)*size, en las 6
    // direcciones, en cualquier parte del tablero y con cualquier origen.
    const float size = 40.f;
    const HexLayout layout(size, {123.f, -45.f});
    const Hex center{2, -1};
    const sf::Vector2f c = layout.hexToPixel(center);

    for (const Hex &dir : hexDirections())
    {
        const sf::Vector2f p = layout.hexToPixel(Hex{center.q + dir.q, center.r + dir.r});
        const float dx = p.x - c.x;
        const float dy = p.y - c.y;
        EXPECT_NEAR(std::sqrt(dx * dx + dy * dy), std::sqrt(3.f) * size, 1e-2f)
            << "direction (" << dir.q << ", " << dir.r << ")";
    }
}

TEST(HexLayout, ResolvesAPixelToTheHexWithTheNearestCenter)
{
    // Esto es lo que quiere decir "en qué casillero hizo clic", y para lo que está
    // el redondeo en coordenadas cúbicas de pixelToHex. La ida y vuelta y los
    // corrimientos chicos siempre caen en el medio del hexágono, donde todas las
    // formas de redondear coinciden; donde dejan de coincidir es en las cuñas cerca
    // de las puntas, que son la mayor parte del área.
    //
    // Los puntos que caen (casi) justo sobre un borde se saltean: ahí cualquiera de
    // las dos respuestas se puede defender.
    const float size = 40.f;
    const HexLayout layout(size, {0.f, 0.f});

    for (float x = -240.f; x <= 240.f; x += 7.f)
    {
        for (float y = -240.f; y <= 240.f; y += 7.f)
        {
            const sf::Vector2f p{x, y};
            Hex nearest{0, 0};
            float best = 1e9f;
            float second = 1e9f;
            for (int q = -8; q <= 8; ++q)
            {
                for (int r = -8; r <= 8; ++r)
                {
                    const sf::Vector2f c = layout.hexToPixel(Hex{q, r});
                    const float dx = c.x - p.x;
                    const float dy = c.y - p.y;
                    const float d = std::sqrt(dx * dx + dy * dy);
                    if (d < best)
                    {
                        second = best;
                        best = d;
                        nearest = Hex{q, r};
                    }
                    else if (d < second)
                    {
                        second = d;
                    }
                }
            }
            if (second - best < 2.f)
            {
                continue;  // on a border between two hexes
            }

            const Hex got = layout.pixelToHex(p);
            EXPECT_TRUE(got == nearest)
                << "pixel (" << x << ", " << y << ") is nearest to (" << nearest.q << ", "
                << nearest.r << ") but resolved to (" << got.q << ", " << got.r << ")";
        }
    }
}
