#include <gtest/gtest.h>
#include "hive/Board.h"
#include "hive/Hex.h"
#include "hive/Piece.h"

#include "support/PrintBoard.h"

// El lado (0,0)->(1,0) está flanqueado por (1,-1) y (0,1): los dos casilleros que
// son vecinos de los dos a la vez. Las direcciones axiales están en neighbors(),
// en Hex.h.

TEST(BoardCanSlide, OpenWhenBothFlankingHexesAreEmpty) {
    Board board;
    Piece mover{Color::White, PieceType::Queen};
    board.place(&mover, Hex{0, 0});

    SCOPED_TRACE(Describe(board));

    EXPECT_TRUE(board.canSlide(Hex{0, 0}, Hex{1, 0}));
}

TEST(BoardCanSlide, OpenWhenOnlyOneFlankingHexIsOccupied) {
    Board board;
    Piece mover{Color::White, PieceType::Queen};
    Piece blocker{Color::Black, PieceType::Ant};
    board.place(&mover, Hex{0, 0});
    board.place(&blocker, Hex{1, -1});  // one flanking hex

    SCOPED_TRACE(Describe(board));

    EXPECT_TRUE(board.canSlide(Hex{0, 0}, Hex{1, 0}));
}

TEST(BoardCanSlide, BlockedWhenBothFlankingHexesAreOccupied) {
    Board board;
    Piece mover{Color::White, PieceType::Queen};
    Piece blockerA{Color::Black, PieceType::Ant};
    Piece blockerB{Color::Black, PieceType::Spider};
    board.place(&mover, Hex{0, 0});
    board.place(&blockerA, Hex{1, -1});
    board.place(&blockerB, Hex{0, 1});

    SCOPED_TRACE(Describe(board));

    EXPECT_FALSE(board.canSlide(Hex{0, 0}, Hex{1, 0}));
}

TEST(BoardCanSlide, OpenInEveryDirectionWhenNoBlockers) {
    // Test de regresión: la primera versión de canSlide acertaba los casilleros que
    // flanquean solo en la dirección (1,0), y de casualidad. Este prueba las 6, así
    // un bug que dependa de la dirección no se puede volver a esconder.
    Board board;
    Piece mover{Color::White, PieceType::Queen};
    board.place(&mover, Hex{0, 0});

    SCOPED_TRACE(Describe(board));

    for (const Hex& to : neighbors(Hex{0, 0})) {
        SCOPED_TRACE(::testing::Message()
                     << "to=(" << to.q << ", " << to.r << ")");
        EXPECT_TRUE(board.canSlide(Hex{0, 0}, to));
    }
}

TEST(BoardCanSlide, BlockedInEveryDirection) {
    // Los pares que flanquean cada una de las 6 direcciones desde el origen,
    // sacados a mano y sin mirar la implementación de Board: si salieran de
    // hexDirections(), un bug ahí y un bug en este test se podrían cancelar sin que
    // nos enteremos.
    struct Case {
        Hex to;
        Hex flank1;
        Hex flank2;
    };
    const std::vector<Case> cases = {
        {Hex{1, 0}, Hex{1, -1}, Hex{0, 1}},
        {Hex{1, -1}, Hex{1, 0}, Hex{0, -1}},
        {Hex{0, -1}, Hex{1, -1}, Hex{-1, 0}},
        {Hex{-1, 0}, Hex{0, -1}, Hex{-1, 1}},
        {Hex{-1, 1}, Hex{-1, 0}, Hex{0, 1}},
        {Hex{0, 1}, Hex{-1, 1}, Hex{1, 0}},
    };

    for (const auto& c : cases) {
        SCOPED_TRACE(::testing::Message()
                     << "to=(" << c.to.q << ", " << c.to.r << ")");

        Board board;
        Piece mover{Color::White, PieceType::Queen};
        Piece blockerA{Color::Black, PieceType::Ant};
        Piece blockerB{Color::Black, PieceType::Spider};
        board.place(&mover, Hex{0, 0});
        board.place(&blockerA, c.flank1);
        board.place(&blockerB, c.flank2);

        SCOPED_TRACE(Describe(board));

        EXPECT_FALSE(board.canSlide(Hex{0, 0}, c.to));
    }
}

TEST(BoardCanSlide, IsSymmetric) {
    Board board;
    Piece mover{Color::White, PieceType::Queen};
    Piece blocker{Color::Black, PieceType::Ant};
    board.place(&mover, Hex{0, 0});
    board.place(&blocker, Hex{1, -1});

    SCOPED_TRACE(Describe(board));

    EXPECT_EQ(board.canSlide(Hex{0, 0}, Hex{1, 0}),
              board.canSlide(Hex{1, 0}, Hex{0, 0}));
}

TEST(BoardCanSlide, IsFalseForADestinationThatIsNotANeighbor) {
    // canSlide solo quiere decir algo sobre el lado entre dos casilleros vecinos. La
    // implementación lo chequea explícitamente (si ninguna dirección coincide,
    // false); este test fija ese chequeo, para que "no existe ese lado" nunca pueda
    // contestar "sí, deslizate tranquila".
    Board board;
    Piece mover{Color::White, PieceType::Queen};
    board.place(&mover, Hex{0, 0});

    SCOPED_TRACE(Describe(board));

    EXPECT_FALSE(board.canSlide(Hex{0, 0}, Hex{2, 0}));   // two hexes away
    EXPECT_FALSE(board.canSlide(Hex{0, 0}, Hex{0, 0}));   // itself
    EXPECT_FALSE(board.canSlide(Hex{0, 0}, Hex{2, -1}));  // distance 2, off-axis
}
