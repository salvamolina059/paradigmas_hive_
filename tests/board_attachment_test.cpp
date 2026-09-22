#include <gtest/gtest.h>
#include "hive/Board.h"
#include "hive/Hex.h"
#include "hive/Piece.h"

#include "support/PrintBoard.h"

TEST(BoardHasOccupiedNeighborExcluding, FalseOnAnEmptyBoard) {
    Board board;
    SCOPED_TRACE(Describe(board));

    EXPECT_FALSE(board.hasOccupiedNeighborExcluding(Hex{0, 0}, nullptr));
}

TEST(BoardHasOccupiedNeighborExcluding, TrueWhenANeighborIsOccupiedAndNotExcluded) {
    Board board;
    Piece other{Color::Black, PieceType::Ant};
    board.place(&other, Hex{1, 0});

    SCOPED_TRACE(Describe(board));

    EXPECT_TRUE(board.hasOccupiedNeighborExcluding(Hex{0, 0}, nullptr));
}

TEST(BoardHasOccupiedNeighborExcluding, FalseWhenTheOnlyOccupiedNeighborIsExcludedAndAlone) {
    Board board;
    Piece mover{Color::White, PieceType::Queen};
    board.place(&mover, Hex{1, 0});

    SCOPED_TRACE(Describe(board));

    EXPECT_FALSE(board.hasOccupiedNeighborExcluding(Hex{0, 0}, &mover));
}

TEST(BoardHasOccupiedNeighborExcluding, TrueWhenTheOnlyOccupiedNeighborIsExcludedButStacked) {
    // Caso de regresión: un escarabajo (el que se mueve) está arriba de una reina (la
    // base) en (1,0). Excluir al escarabajo no tiene que dejar (1,0) vacío: la reina
    // sigue abajo, así que (0,0) sigue tocando la colmena.
    Board board;
    Piece base{Color::White, PieceType::Queen};
    Piece mover{Color::Black, PieceType::Beetle};
    board.place(&base, Hex{1, 0});
    board.place(&mover, Hex{1, 0});  // stacks on top of base

    SCOPED_TRACE(Describe(board));

    EXPECT_TRUE(board.hasOccupiedNeighborExcluding(Hex{0, 0}, &mover));
}

TEST(BoardHasOccupiedNeighborExcluding, ExcludingAPieceNotAmongNeighborsStillCountsOthers) {
    Board board;
    Piece other{Color::Black, PieceType::Ant};
    Piece unrelated{Color::White, PieceType::Spider};
    board.place(&other, Hex{1, 0});
    board.place(&unrelated, Hex{5, 5});  // far away, not a neighbor of (0,0)

    SCOPED_TRACE(Describe(board));

    EXPECT_TRUE(board.hasOccupiedNeighborExcluding(Hex{0, 0}, &unrelated));
}

TEST(BoardWouldStayAttached, TrueWhenMoverIsTheOnlyPieceOnTheBoard) {
    // Caso vacío: no hay nada más a lo que quedar pegada, así que en lo que respecta
    // al contacto con la colmena, cualquier destino sirve.
    Board board;
    Piece mover{Color::White, PieceType::Queen};
    board.place(&mover, Hex{0, 0});

    SCOPED_TRACE(Describe(board));

    EXPECT_TRUE(board.wouldStayAttached(Hex{1, 0}, mover));
}

TEST(BoardWouldStayAttached, TrueWhenDestinationTouchesAnotherPiece) {
    Board board;
    Piece mover{Color::White, PieceType::Queen};
    Piece other{Color::Black, PieceType::Ant};
    board.place(&mover, Hex{0, 0});
    board.place(&other, Hex{1, -1});

    SCOPED_TRACE(Describe(board));

    EXPECT_TRUE(board.wouldStayAttached(Hex{1, 0}, mover));
}

TEST(BoardWouldStayAttached, FalseWhenNotAloneAndDestinationTouchesNothing) {
    // El mismo contraejemplo que en los tests de la reina: si hay otra pieza en el
    // tablero, un destino que solo toca el casillero que la pieza está por dejar no
    // está realmente pegado a nada.
    Board board;
    Piece mover{Color::White, PieceType::Queen};
    Piece other{Color::Black, PieceType::Ant};
    board.place(&mover, Hex{0, 0});
    board.place(&other, Hex{-1, 0});

    SCOPED_TRACE(Describe(board));

    EXPECT_FALSE(board.wouldStayAttached(Hex{1, 0}, mover));
}
