#include <gtest/gtest.h>
#include "hive/Board.h"
#include "hive/Hex.h"
#include "hive/Piece.h"

#include "support/PrintBoard.h"

#include <algorithm>
#include <vector>

namespace {

bool Contains(const std::vector<Hex>& hexes, const Hex& target) {
    return std::find(hexes.begin(), hexes.end(), target) != hexes.end();
}

}  // namespace

TEST(BoardLegalPlacementSpots, OnAnEmptyBoardOnlyTheOriginIsLegal) {
    Board board;
    const auto spots = board.legalPlacementSpots(Color::White);

    SCOPED_TRACE(Describe(board) + Describe(spots, "spots"));

    ASSERT_EQ(spots.size(), 1u);
    EXPECT_TRUE(Contains(spots, Hex{0, 0}));
}

TEST(BoardLegalPlacementSpots, WithOneLonePieceAllSixNeighborsAreLegal) {
    // Segunda colocación de la partida: tiene que ir pegada a la única pieza, y la
    // regla de no tocar al rival se saltea (esa única pieza es necesariamente del
    // rival).
    Board board;
    Piece white{Color::White, PieceType::Queen};
    board.place(&white, Hex{0, 0});

    const auto spots = board.legalPlacementSpots(Color::Black);

    SCOPED_TRACE(Describe(board) + Describe(spots, "spots"));

    ASSERT_EQ(spots.size(), 6u);
    for (const auto& n : neighbors(Hex{0, 0})) {
        EXPECT_TRUE(Contains(spots, n));
    }
}

TEST(BoardLegalPlacementSpots, ExcludesHexesTouchingAnEnemyPiece) {
    // Blancas en (0,0) y (1,0); negras en (2,0). Verificado aparte: los lugares
    // legales de las blancas son exactamente los 5 casilleros que bordean (0,0) y no
    // tocan (2,0); los de las negras, exactamente los 3 que bordean (2,0) y no tocan
    // ninguna pieza blanca.
    Board board;
    Piece w1{Color::White, PieceType::Queen};
    Piece w2{Color::White, PieceType::Ant};
    Piece b1{Color::Black, PieceType::Spider};
    board.place(&w1, Hex{0, 0});
    board.place(&w2, Hex{1, 0});
    board.place(&b1, Hex{2, 0});

    const auto whiteSpots = board.legalPlacementSpots(Color::White);
    const auto blackSpots = board.legalPlacementSpots(Color::Black);

    const std::vector<Hex> expectedWhite = {
        Hex{-1, 0}, Hex{-1, 1}, Hex{0, -1}, Hex{0, 1}, Hex{1, -1},
    };
    SCOPED_TRACE(Describe(board) + Describe(whiteSpots, "whiteSpots")
                 + Describe(blackSpots, "blackSpots"));

    ASSERT_EQ(whiteSpots.size(), expectedWhite.size());
    for (const auto& h : expectedWhite) {
        EXPECT_TRUE(Contains(whiteSpots, h));
    }

    const std::vector<Hex> expectedBlack = {
        Hex{2, 1}, Hex{3, -1}, Hex{3, 0},
    };
    ASSERT_EQ(blackSpots.size(), expectedBlack.size());
    for (const auto& h : expectedBlack) {
        EXPECT_TRUE(Contains(blackSpots, h));
    }
}

TEST(BoardLegalPlacementSpots, EnemyAdjacencyIsDeterminedByTheTopOfTheStackNotWhatsBuried) {
    // Reina negra en (0,0), con un escarabajo blanco arriba: la pieza de arriba es
    // blanca. La araña negra lejana en (5,5) está solo para que occupiedHexes() tenga
    // tamaño 2 y corra la rama normal, en vez de la excepción del único casillero.
    //
    // (1,0) toca la pila y nada más, no toca la pieza lejana. Para la regla de no
    // tocar al rival, ese casillero pertenece a quien esté ARRIBA de la pila: legal
    // para las blancas (arriba hay una pieza propia, aunque abajo esté la reina
    // negra) e ilegal para las negras (arriba hay una pieza rival, aunque la
    // enterrada sea de ellas).
    Board board;
    Piece blackQueen{Color::Black, PieceType::Queen};
    Piece whiteBeetle{Color::White, PieceType::Beetle};
    Piece blackSpider{Color::Black, PieceType::Spider};
    board.place(&blackQueen, Hex{0, 0});
    board.place(&whiteBeetle, Hex{0, 0});  // stacks on top
    board.place(&blackSpider, Hex{5, 5});

    SCOPED_TRACE(Describe(board));

    EXPECT_TRUE(Contains(board.legalPlacementSpots(Color::White), Hex{1, 0}));
    EXPECT_FALSE(Contains(board.legalPlacementSpots(Color::Black), Hex{1, 0}));
}

TEST(BoardLegalPlacementSpots, NeverIncludesAnAlreadyOccupiedHex) {
    Board board;
    Piece white{Color::White, PieceType::Queen};
    board.place(&white, Hex{0, 0});

    const auto spots = board.legalPlacementSpots(Color::White);

    SCOPED_TRACE(Describe(board) + Describe(spots, "spots"));

    EXPECT_FALSE(Contains(spots, Hex{0, 0}));
}
