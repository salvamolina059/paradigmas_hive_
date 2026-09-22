#include <gtest/gtest.h>
#include "hive/Board.h"
#include "hive/Hex.h"
#include "hive/Piece.h"
#include "hive/movement/AntMovement.h"

#include "support/PrintBoard.h"

#include <algorithm>
#include <vector>

namespace {

bool Contains(const std::vector<Hex>& hexes, const Hex& target) {
    return std::find(hexes.begin(), hexes.end(), target) != hexes.end();
}

}  // namespace

TEST(AntMovement, ReachesFarBeyondThreeStepsAlongAnOpenWall) {
    // El mismo fixture de pared que los tests de la araña: (0,0)..(4,0) y la hormiga
    // en (0,-1). A diferencia de la araña, que hace exactamente 3 pasos y llega como
    // máximo a (3,-1), la hormiga no tiene tope y tiene que llegar a (4,-1): 4 saltos,
    // uno más allá del techo de la araña.
    Board board;
    Piece ant{Color::White, PieceType::Ant};
    Piece w0{Color::Black, PieceType::Queen};
    Piece w1{Color::Black, PieceType::Queen};
    Piece w2{Color::Black, PieceType::Queen};
    Piece w3{Color::Black, PieceType::Queen};
    Piece w4{Color::Black, PieceType::Queen};
    board.place(&ant, Hex{0, -1});
    board.place(&w0, Hex{0, 0});
    board.place(&w1, Hex{1, 0});
    board.place(&w2, Hex{2, 0});
    board.place(&w3, Hex{3, 0});
    board.place(&w4, Hex{4, 0});

    AntMovement movement;
    const auto moves = movement.moves(board, Hex{0, -1}, ant);

    SCOPED_TRACE(Describe(board) + Describe(moves, "moves"));

    EXPECT_TRUE(Contains(moves, Hex{4, -1}));
}

TEST(AntMovement, DoesNotIncludeOccupiedHexesOrItsOwnStartingHex) {
    Board board;
    Piece ant{Color::White, PieceType::Ant};
    Piece other{Color::Black, PieceType::Queen};
    board.place(&ant, Hex{0, 0});
    board.place(&other, Hex{1, 0});

    AntMovement movement;
    const auto moves = movement.moves(board, Hex{0, 0}, ant);

    SCOPED_TRACE(Describe(board) + Describe(moves, "moves"));

    EXPECT_FALSE(Contains(moves, Hex{1, 0}));  // occupied
    EXPECT_FALSE(Contains(moves, Hex{0, 0}));  // its own starting hex
}

// --- el hueco adentro de un anillo: no alcanza con ser vecino -----------

TEST(AntMovement, CannotEnterTheHoleInsideARingOfPieces) {
    // 6 piezas en los vecinos del origen forman un anillo cerrado; el origen queda
    // vacío, y es el "hueco". La hormiga arranca afuera del anillo, pegada a una de
    // las piezas.
    //
    // El hueco SÍ es vecino de casilleros ocupados: lo tocan las 6 piezas del anillo.
    // Una implementación que calcula "el borde" como "cualquier casillero vacío que
    // toque la colmena", en vez de simular la caminata paso a paso desde donde está la
    // hormiga, lo incluiría mal. No tiene ninguna entrada legal: todos los vecinos del
    // hueco son piezas del anillo, así que la hormiga nunca puede ni pararse al lado
    // para meterse.
    //
    // Eso sí, la hormiga PUEDE caminar toda la vuelta por afuera del anillo y llegar a
    // casilleros del otro lado.
    Board board;
    Piece ant{Color::White, PieceType::Ant};
    Piece r0{Color::Black, PieceType::Queen};
    Piece r1{Color::Black, PieceType::Queen};
    Piece r2{Color::Black, PieceType::Queen};
    Piece r3{Color::Black, PieceType::Queen};
    Piece r4{Color::Black, PieceType::Queen};
    Piece r5{Color::Black, PieceType::Queen};
    board.place(&r0, Hex{1, 0});
    board.place(&r1, Hex{1, -1});
    board.place(&r2, Hex{0, -1});
    board.place(&r3, Hex{-1, 0});
    board.place(&r4, Hex{-1, 1});
    board.place(&r5, Hex{0, 1});
    board.place(&ant, Hex{2, 0});  // outside, adjacent to r0

    AntMovement movement;
    const auto moves = movement.moves(board, Hex{2, 0}, ant);

    SCOPED_TRACE(Describe(board) + Describe(moves, "moves"));

    EXPECT_FALSE(Contains(moves, Hex{0, 0}))
        << "the hole inside the ring must not be reachable";
    // Control: la hormiga no está devolviendo nada de nada; puede caminar por afuera
    // del anillo, incluso hasta el otro lado.
    EXPECT_TRUE(Contains(moves, Hex{-2, 0}))
        << "far side of the ring, reached by going around the outside";
}

TEST(AntMovement, NeverOffersAnOccupiedHexEvenDeepInsideTheHive) {
    // Va de la mano de DoesNotIncludeOccupiedHexesOrItsOwnStartingHex, que descarta
    // (1,0) en un tablero de dos piezas: ahí un (1,0) ocupado igual queda descartado
    // por la regla del contacto —su único vecino ocupado sería la propia hormiga—, así
    // que el chequeo de ocupación no es el que hace el trabajo.
    //
    // En la pared de 5 piezas, cada casillero de la pared toca otra pieza de la pared y
    // se alcanza por un hueco abierto, así que lo único que mantiene a la hormiga
    // afuera es que están ocupados.
    Board board;
    Piece ant{Color::White, PieceType::Ant};
    Piece w0{Color::Black, PieceType::Queen};
    Piece w1{Color::Black, PieceType::Queen};
    Piece w2{Color::Black, PieceType::Queen};
    Piece w3{Color::Black, PieceType::Queen};
    Piece w4{Color::Black, PieceType::Queen};
    board.place(&ant, Hex{0, -1});
    board.place(&w0, Hex{0, 0});
    board.place(&w1, Hex{1, 0});
    board.place(&w2, Hex{2, 0});
    board.place(&w3, Hex{3, 0});
    board.place(&w4, Hex{4, 0});

    AntMovement movement;
    const auto moves = movement.moves(board, Hex{0, -1}, ant);

    SCOPED_TRACE(Describe(board) + Describe(moves, "moves"));

    ASSERT_FALSE(moves.empty());
    for (const auto& h : moves) {
        EXPECT_FALSE(board.isOccupied(h))
            << "offered occupied hex (" << h.q << ", " << h.r << ")";
    }
}

TEST(AntMovement, CannotSqueezeThroughAClosedGateIntoAPocket) {
    // El test del anillo de arriba muestra que la hormiga no llega a un hueco sin
    // ninguna entrada. Este es el caso más fino: el hueco SÍ tiene entrada, y la
    // hormiga puede pararse justo ahí —(0,1) está vacío y se alcanza desde afuera—,
    // pero el paso (0,1) -> (0,0) está flanqueado por (-1,1) y (1,0), los dos
    // ocupados, así que no puede deslizarse.
    //
    // Anillo de 5 piezas, con un hueco en (0,1), alrededor del origen vacío; la hormiga
    // afuera. A diferencia del anillo cerrado, acá lo único que la puede mantener
    // afuera de (0,0) es canSlide: la adyacencia y el contacto con la colmena los
    // cumple.
    Board board;
    Piece ant{Color::White, PieceType::Ant};
    Piece r0{Color::Black, PieceType::Queen};
    Piece r1{Color::Black, PieceType::Queen};
    Piece r2{Color::Black, PieceType::Queen};
    Piece r3{Color::Black, PieceType::Queen};
    Piece r4{Color::Black, PieceType::Queen};
    board.place(&r0, Hex{1, 0});
    board.place(&r1, Hex{1, -1});
    board.place(&r2, Hex{0, -1});
    board.place(&r3, Hex{-1, 0});
    board.place(&r4, Hex{-1, 1});
    board.place(&ant, Hex{2, 0});  // outside, adjacent to r0

    AntMovement movement;
    const auto moves = movement.moves(board, Hex{2, 0}, ant);

    SCOPED_TRACE(Describe(board) + Describe(moves, "moves"));

    EXPECT_TRUE(Contains(moves, Hex{0, 1}))
        << "the gap in the ring is an ordinary perimeter hex";
    EXPECT_FALSE(Contains(moves, Hex{0, 0}))
        << "the pocket is only entered by squeezing through a closed gate";
}
