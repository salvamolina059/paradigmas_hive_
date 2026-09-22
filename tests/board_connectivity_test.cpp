#include <gtest/gtest.h>
#include "hive/Board.h"
#include "hive/Hex.h"
#include "hive/Piece.h"

#include "support/PrintBoard.h"

// --- isConnected() ------------------------------------------------------

TEST(BoardConnectivity, EmptyBoardIsConnected) {
    Board board;
    SCOPED_TRACE(Describe(board));

    EXPECT_TRUE(board.isConnected());
}

TEST(BoardConnectivity, SingleOccupiedHexIsConnected) {
    Board board;
    Piece piece{Color::White, PieceType::Queen};
    board.place(&piece, Hex{0, 0});

    SCOPED_TRACE(Describe(board));

    EXPECT_TRUE(board.isConnected());
}

TEST(BoardConnectivity, ChainOfHexesIsConnected) {
    Board board;
    Piece a{Color::White, PieceType::Queen};
    Piece b{Color::Black, PieceType::Ant};
    Piece c{Color::White, PieceType::Spider};
    board.place(&a, Hex{0, 0});
    board.place(&b, Hex{1, 0});
    board.place(&c, Hex{2, 0});

    SCOPED_TRACE(Describe(board));

    EXPECT_TRUE(board.isConnected());
}

TEST(BoardConnectivity, DisconnectedHexesAreNotConnected) {
    Board board;
    Piece a{Color::White, PieceType::Queen};
    Piece b{Color::Black, PieceType::Ant};
    board.place(&a, Hex{0, 0});
    board.place(&b, Hex{5, 5});

    SCOPED_TRACE(Describe(board));

    EXPECT_FALSE(board.isConnected());
}

// --- canMove(): la regla de la colmena, del lado del casillero de partida ---

TEST(BoardCanMove, TheOnlyPieceOnTheBoardCanAlwaysMove) {
    Board board;
    Piece piece{Color::White, PieceType::Queen};
    board.place(&piece, Hex{0, 0});

    SCOPED_TRACE(Describe(board));

    EXPECT_TRUE(board.canMove(Hex{0, 0}));
}

TEST(BoardCanMove, ACutHexIsPinned) {
    // Cadena derecha: (0,0) - (1,0) - (2,0). El casillero del medio es el único
    // vínculo entre las dos puntas, así que no se puede mover sin partir la colmena:
    // está clavado.
    Board board;
    Piece a{Color::White, PieceType::Queen};
    Piece b{Color::Black, PieceType::Ant};
    Piece c{Color::White, PieceType::Spider};
    board.place(&a, Hex{0, 0});
    board.place(&b, Hex{1, 0});
    board.place(&c, Hex{2, 0});

    SCOPED_TRACE(Describe(board));

    EXPECT_FALSE(board.canMove(Hex{1, 0}));
    EXPECT_TRUE(board.canMove(Hex{0, 0}));
    EXPECT_TRUE(board.canMove(Hex{2, 0}));
}

TEST(BoardCanMove, ANonCutHexCanMove) {
    // (0,0), (1,0) y (1,-1) son vecinos entre sí, así que saquemos el que saquemos,
    // los otros dos siguen conectados.
    Board board;
    Piece a{Color::White, PieceType::Queen};
    Piece b{Color::Black, PieceType::Ant};
    Piece c{Color::White, PieceType::Spider};
    board.place(&a, Hex{0, 0});
    board.place(&b, Hex{1, 0});
    board.place(&c, Hex{1, -1});

    SCOPED_TRACE(Describe(board));

    EXPECT_TRUE(board.canMove(Hex{0, 0}));
    EXPECT_TRUE(board.canMove(Hex{1, 0}));
    EXPECT_TRUE(board.canMove(Hex{1, -1}));
}

TEST(BoardCanMove, TopOfAStackCanAlwaysMove) {
    // Aunque (0,0) sea un punto de corte, una pieza apilada arriba no está clavada:
    // el casillero sigue ocupado por la que está abajo.
    Board board;
    Piece a{Color::White, PieceType::Queen};
    Piece b{Color::Black, PieceType::Ant};
    Piece c{Color::White, PieceType::Spider};
    Piece climber{Color::Black, PieceType::Beetle};
    board.place(&a, Hex{0, 0});
    board.place(&b, Hex{1, 0});
    board.place(&c, Hex{2, 0});
    board.place(&climber, Hex{3, 0});
    board.move(Hex{3, 0}, Hex{1, 0});  // climber stacks on the cut hex

    SCOPED_TRACE(Describe(board));

    EXPECT_TRUE(board.canMove(Hex{1, 0}));
}
