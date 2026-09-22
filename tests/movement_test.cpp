#include <gtest/gtest.h>
#include "hive/Board.h"
#include "hive/Hex.h"
#include "hive/Piece.h"
#include "hive/movement/BeetleMovement.h"

#include "support/PrintBoard.h"

#include <algorithm>
#include <vector>

// ============================================================================
// Tests propios.
//
// El resto de tests/ son los tests con los que se corrige. Cuando uno falla
// imprime el tablero y la lista que devolvió su código, así que dicen bastante
// más que "esperaba true": ver PrintBoard.h. Pero los armó otra persona, y
// muchos usan tableros grandes —paredes de cinco piezas, anillos cerrados—
// elegidos para tapar agujeros, no para explicar una regla.
//
// Este archivo es para los tests propios, los que se escriben mientras se
// implementa. Arrancan al revés: un tablero chico, armado a mano, que aísla UNA
// regla. Cuando algo falla, ese es el que dice cuál de todas se rompió.
//
// No se corrige, y no hace falta que lo entreguen. Igual conviene escribirlos:
// una vez encontrado un bug, dejar el test escrito evita que vuelva.
//
// Abajo hay un ejemplo resuelto y después una lista de situaciones sin
// resolver. Completen las que sirvan, agreguen otras, borren las que no.
// ============================================================================

namespace
{

    bool Contains(const std::vector<Hex> &hexes, const Hex &target)
    {
        return std::find(hexes.begin(), hexes.end(), target) != hexes.end();
    }

} // namespace

// --- Ejemplo resuelto ---------------------------------------------------
//
// Un tablero se arma colocando piezas sueltas: no hace falta jugar una
// partida entera para probar una regla. Miren el SCOPED_TRACE: si el test
// falla, imprime el tablero y las movidas en vez de solo "esperaba true". Vale
// para todos los asserts que vengan abajo, así que va una vez, después de armar
// el tablero y calcular el resultado. (Para colgarlo de un assert suelto
// existe también `<< Describe(board)`; los dos están en PrintBoard.h.)
//
// Este ejemplo falla hasta que el escarabajo esté implementado. Es lo que
// tiene que pasar: el test está bien, lo que falta es el código.

TEST(MisTests, EscarabajoSeSubeSobreLaPiezaVecina)
{
    Board board;
    Piece beetle{Color::White, PieceType::Beetle};
    Piece vecina{Color::Black, PieceType::Queen};
    board.place(&beetle, Hex{0, 0});
    board.place(&vecina, Hex{1, 0});

    const auto moves = BeetleMovement().moves(board, Hex{0, 0}, beetle);

    SCOPED_TRACE(Describe(board) + Describe(moves, "moves"));

    EXPECT_TRUE(Contains(moves, Hex{1, 0}))
        << "el escarabajo debería poder subirse a (1, 0)";
}

// --- Situaciones para cubrir --------------------------------------------
//
// Cada una es un tablero distinto, no un assert más sobre el mismo. GTEST_SKIP
// hace que aparezcan como SKIPPED y no como si ya estuvieran hechas; cuando
// completen una, borren esa línea.

TEST(MisTests, ReinaNoSeSubeSobreUnaPiezaVecina)
{
    GTEST_SKIP() << "TODO";
}

TEST(MisTests, ReinaNoPasaPorUnHuecoTapadoDeLosDosLados)
{
    GTEST_SKIP() << "TODO";
}

TEST(MisTests, EscarabajoSoloEnElTableroSeMueveALosSeisVecinos)
{
    GTEST_SKIP() << "TODO";
}

TEST(MisTests, EscarabajoArribaDeUnaPilaBajaAUnCasilleroVacio)
{
    GTEST_SKIP() << "TODO";
}

TEST(MisTests, SaltamontesSaltaVariasPiezasSeguidas)
{
    GTEST_SKIP() << "TODO";
}

TEST(MisTests, SaltamontesNoSaltaSiElVecinoEstaVacio)
{
    GTEST_SKIP() << "TODO";
}

TEST(MisTests, LaFabricaDevuelveUnaEstrategiaDistintaPorTipo)
{
    GTEST_SKIP() << "TODO";
}

TEST(MisTests, ElAdaptadorNoVeElCasilleroIgnorado)
{
    GTEST_SKIP() << "TODO";
}

TEST(MisTests, UnaPiezaQuePartiriaLaColmenaNoSePuedeMover)
{
    GTEST_SKIP() << "TODO";
}

TEST(MisTests, LaVaquitaSiempreTerminaEnUnCasilleroVacio)
{
    GTEST_SKIP() << "TODO";
}
