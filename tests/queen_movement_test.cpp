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

TEST(QueenMovement, AloneOnTheBoardCanMoveToAllSixNeighbors) {
    // Legal a pesar de la regla del contacto: si no hay ninguna otra pieza en el
    // tablero, no hay a qué quedar pegada.
    Board board;
    Piece queen{Color::White, PieceType::Queen};
    board.place(&queen, Hex{0, 0});

    QueenMovement movement;
    const auto moves = movement.moves(board, Hex{0, 0}, queen);

    SCOPED_TRACE(Describe(board) + Describe(moves, "moves"));

    ASSERT_EQ(moves.size(), 6u);
    for (const auto& n : neighbors(Hex{0, 0})) {
        EXPECT_TRUE(Contains(moves, n));
    }
}

TEST(QueenMovement, CannotMoveToAHexDisconnectedFromTheRestOfTheHive) {
    // El contraejemplo que motivó hasOccupiedNeighborExcluding: con la única otra
    // pieza en (-1,0), o sea diametralmente opuesta, (1,0) está vacío y pasa canSlide
    // —físicamente nada lo tapa—, pero no toca (-1,0) ni nada más. Irse ahí dejaría a
    // la reina sola, como un grupo separado del resto.
    Board board;
    Piece queen{Color::White, PieceType::Queen};
    Piece other{Color::Black, PieceType::Ant};
    board.place(&queen, Hex{0, 0});
    board.place(&other, Hex{-1, 0});

    QueenMovement movement;
    const auto moves = movement.moves(board, Hex{0, 0}, queen);

    SCOPED_TRACE(Describe(board) + Describe(moves, "moves"));

    EXPECT_FALSE(Contains(moves, Hex{1, 0}));
}

TEST(QueenMovement, CannotMoveOntoAnOccupiedNeighborOrLoseContact) {
    // Con una sola otra pieza en (1,0), los únicos casilleros que siguen tocándola
    // después de que la reina deja (0,0) son los dos que flanquean el lado
    // origen->(1,0): (1,-1) y (0,1). Los otros 3 vecinos vacíos pasan canSlide —nada
    // los tapa— pero dejarían a la reina sin nada al lado, rompiendo la regla de la
    // colmena.
    Board board;
    Piece queen{Color::White, PieceType::Queen};
    Piece other{Color::Black, PieceType::Ant};
    board.place(&queen, Hex{0, 0});
    board.place(&other, Hex{1, 0});

    QueenMovement movement;
    const auto moves = movement.moves(board, Hex{0, 0}, queen);

    SCOPED_TRACE(Describe(board) + Describe(moves, "moves"));

    ASSERT_EQ(moves.size(), 2u);
    EXPECT_TRUE(Contains(moves, Hex{1, -1}));
    EXPECT_TRUE(Contains(moves, Hex{0, 1}));
}

TEST(QueenMovement, CannotSlideThroughAGateBlockedOnBothSides) {
    // El destino (1,0) está vacío, pero los dos casilleros que flanquean el lado
    // origen->(1,0) —(1,-1) y (0,1)— están ocupados, así que la reina no pasa por ese
    // hueco aunque (1,0) esté libre. De los vecinos vacíos que quedan, solo (0,-1) y
    // (-1,1) tocan alguna de las dos piezas que tapan; (-1,0) no toca ninguna y
    // dejaría a la reina suelta, así que también queda afuera.
    Board board;
    Piece queen{Color::White, PieceType::Queen};
    Piece blockerA{Color::Black, PieceType::Ant};
    Piece blockerB{Color::Black, PieceType::Spider};
    board.place(&queen, Hex{0, 0});
    board.place(&blockerA, Hex{1, -1});
    board.place(&blockerB, Hex{0, 1});

    QueenMovement movement;
    const auto moves = movement.moves(board, Hex{0, 0}, queen);

    SCOPED_TRACE(Describe(board) + Describe(moves, "moves"));

    EXPECT_FALSE(Contains(moves, Hex{1, 0}));
    EXPECT_FALSE(Contains(moves, Hex{-1, 0}));
    ASSERT_EQ(moves.size(), 2u);
    EXPECT_TRUE(Contains(moves, Hex{0, -1}));
    EXPECT_TRUE(Contains(moves, Hex{-1, 1}));
}

TEST(QueenMovement, ReturnsEmptyWhenFullySurrounded) {
    Board board;
    Piece queen{Color::White, PieceType::Queen};
    board.place(&queen, Hex{0, 0});

    std::vector<Piece> ring;
    for (int i = 0; i < 6; ++i) {
        ring.push_back(Piece{Color::Black, PieceType::Ant});
    }
    const auto ringHexes = neighbors(Hex{0, 0});
    for (std::size_t i = 0; i < ringHexes.size(); ++i) {
        board.place(&ring[i], ringHexes[i]);
    }

    QueenMovement movement;
    const auto moves = movement.moves(board, Hex{0, 0}, queen);

    SCOPED_TRACE(Describe(board) + Describe(moves, "moves"));

    EXPECT_TRUE(moves.empty());
}

TEST(QueenMovement, NeverOffersAnOccupiedNeighborEvenWhenItPassesEveryOtherCheck) {
    // La reina mira únicamente vecinos VACÍOS: es lo único que la separa del
    // escarabajo. Los otros fixtures no fijan eso: con una sola pieza más en (1,0),
    // ese casillero igual queda descartado por la regla del contacto (su único vecino
    // ocupado sería la propia reina), así que una implementación que se olvidó de
    // filtrar los ocupados también los pasaría.
    //
    // Acá (1,0) sobrevive a todos los demás filtros —el hueco está abierto, los dos
    // casilleros que flanquean están vacíos, y sigue pegado a la colmena a través de
    // (2,0), que no es la reina—, así que lo único que puede evitar que la reina se le
    // suba es el chequeo de que esté vacío.
    Board board;
    Piece queen{Color::White, PieceType::Queen};
    Piece other{Color::Black, PieceType::Ant};
    Piece far{Color::Black, PieceType::Spider};
    board.place(&queen, Hex{0, 0});
    board.place(&other, Hex{1, 0});
    board.place(&far, Hex{2, 0});

    QueenMovement movement;
    const auto moves = movement.moves(board, Hex{0, 0}, queen);

    SCOPED_TRACE(Describe(board) + Describe(moves, "moves"));

    EXPECT_FALSE(Contains(moves, Hex{1, 0})) << "queens do not climb";
    ASSERT_EQ(moves.size(), 2u);
    EXPECT_TRUE(Contains(moves, Hex{1, -1}));
    EXPECT_TRUE(Contains(moves, Hex{0, 1}));
}
