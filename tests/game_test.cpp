#include <gtest/gtest.h>
#include "hive/Game.h"

#include <algorithm>
#include <functional>

#include "hive/movement/BeetleMovement.h"

namespace {

// Aplica la primera movida legal que cumpla `predicate`. Devuelve si encontró y aplicó
// una; quien la llama chequea ese valor con un ASSERT, así el test corta ahí mismo si
// la movida que necesitaba no estaba.
bool ApplyFirstMatching(Game& game, const std::function<bool(const Move&)>& predicate) {
    for (const auto& move : game.legalMoves()) {
        if (predicate(move)) {
            return game.applyMove(move);
        }
    }
    return false;
}

bool IsPlacementOfType(const Move& move, PieceType type) {
    const auto* p = std::get_if<Placement>(&move);
    return p && p->type == type;
}

bool IsNonQueenPlacement(const Move& move) {
    const auto* p = std::get_if<Placement>(&move);
    return p && p->type != PieceType::Queen;
}

bool IsPlacementOfTypeAdjacentTo(const Move& move, PieceType type, const Hex& target) {
    const auto* p = std::get_if<Placement>(&move);
    return p && p->type == type && distance(p->destination, target) == 1;
}

bool IsPlacementOfTypeAdjacentToBoth(const Move& move, PieceType type, const Hex& a,
                                      const Hex& b) {
    const auto* p = std::get_if<Placement>(&move);
    return p && p->type == type && distance(p->destination, a) == 1 &&
           distance(p->destination, b) == 1;
}

bool IsNonQueenPlacementAdjacentTo(const Move& move, const Hex& target) {
    const auto* p = std::get_if<Placement>(&move);
    return p && p->type != PieceType::Queen && distance(p->destination, target) == 1;
}

bool IsNonQueenPlacementAt(const Move& move, const Hex& target) {
    const auto* p = std::get_if<Placement>(&move);
    return p && p->type != PieceType::Queen && p->destination == target;
}

bool IsMovementAdjacentTo(const Move& move, const Hex& target) {
    const auto* m = std::get_if<Movement>(&move);
    return m && distance(m->destination, target) == 1;
}

bool Contains(const std::vector<Hex>& hexes, const Hex& target) {
    return std::find(hexes.begin(), hexes.end(), target) != hexes.end();
}

}  // namespace


TEST(Game, StartsWithWhiteToMoveOnTurnOne) {
    Game game;

    EXPECT_EQ(game.turn(), Color::White);
    EXPECT_EQ(game.turnNumber(), 1);
}

TEST(Game, StartsOngoingWithNoWinner) {
    Game game;

    EXPECT_EQ(game.status(), GameStatus::Ongoing);
    EXPECT_EQ(game.winner(), std::nullopt);
}

TEST(Game, StartsWithAnEmptyBoard) {
    Game game;

    EXPECT_TRUE(game.board().occupiedHexes().empty());
}

TEST(Game, BothPlayersStartWithFullHands) {
    Game game;

    EXPECT_EQ(game.player(Color::White).remaining(PieceType::Queen), 1);
    EXPECT_EQ(game.player(Color::Black).remaining(PieceType::Queen), 1);
    EXPECT_FALSE(game.player(Color::White).hasPlacedQueen());
    EXPECT_FALSE(game.player(Color::Black).hasPlacedQueen());
}

// --- legalMoves() / applyMove(): solo colocaciones -----------------------

TEST(Game, FirstTurnOffersAllEightTypesAtTheOrigin) {
    Game game;
    const auto moves = game.legalMoves();

    ASSERT_EQ(moves.size(), 8u);
    for (const auto& move : moves) {
        ASSERT_TRUE(std::holds_alternative<Placement>(move));
        const auto& placement = std::get<Placement>(move);
        EXPECT_EQ(placement.destination.q, 0);
        EXPECT_EQ(placement.destination.r, 0);
    }
}

TEST(Game, ApplyingALegalPlacementMutatesStateAndAdvancesTurn) {
    Game game;
    const auto moves = game.legalMoves();
    // Chequeado en vez de asumido: llamar a front() sobre un vector vacío aborta el
    // binario entero, y entonces "Player todavía no está implementado" parece un
    // compilador roto. Un test tiene que fallar, no llevarse puesta la suite.
    ASSERT_FALSE(moves.empty());
    const Move move = moves.front();
    const auto& placement = std::get<Placement>(move);
    const int handBefore = game.player(Color::White).remaining(placement.type);

    ASSERT_TRUE(game.applyMove(move));

    EXPECT_EQ(game.board().stackHeight(placement.destination), 1u);
    Piece* placed = game.board().topAt(placement.destination);
    ASSERT_NE(placed, nullptr);
    EXPECT_EQ(placed->color, Color::White);
    EXPECT_EQ(placed->type, placement.type);

    EXPECT_EQ(game.player(Color::White).remaining(placement.type), handBefore - 1);
    EXPECT_EQ(game.turn(), Color::Black);
    EXPECT_EQ(game.turnNumber(), 2);
    EXPECT_EQ(game.lastMoved(), placed);  // the placed piece replaces lastMoved_
}

TEST(Game, ApplyMoveRejectsAnIllegalMove) {
    Game game;
    const Move illegal = Placement{PieceType::Queen, Hex{5, 5}};  // not the origin

    EXPECT_FALSE(game.applyMove(illegal));
    EXPECT_TRUE(game.board().occupiedHexes().empty());
    EXPECT_EQ(game.turn(), Color::White);
    EXPECT_EQ(game.turnNumber(), 1);
}

TEST(Game, QueenByTurnFourRestrictsPlacementToQueenOnly) {
    Game game;

    // Colocamos una pieza que no sea la reina 6 veces, alternando colores y usando lo
    // que ofrezca legalMoves(), para llegar al 4º turno de las blancas (turnNumber
    // global 7) sin que ninguno de los dos haya colocado su reina.
    for (int i = 0; i < 6; ++i) {
        const auto moves = game.legalMoves();
        bool applied = false;
        for (const auto& move : moves) {
            const auto& placement = std::get<Placement>(move);
            if (placement.type != PieceType::Queen) {
                ASSERT_TRUE(game.applyMove(move));
                applied = true;
                break;
            }
        }
        ASSERT_TRUE(applied);
    }

    ASSERT_EQ(game.turnNumber(), 7);
    ASSERT_EQ(game.turn(), Color::White);
    ASSERT_FALSE(game.player(Color::White).hasPlacedQueen());

    const auto moves = game.legalMoves();
    ASSERT_FALSE(moves.empty());
    for (const auto& move : moves) {
        const auto& placement = std::get<Placement>(move);
        EXPECT_EQ(placement.type, PieceType::Queen);
    }
}

// --- legalMoves() / applyMove(): movimientos ---------------------------

TEST(Game, NoMovementOptionsBeforeTheQueenIsPlaced) {
    Game game;

    // Turno 1: las blancas colocan una pieza que no es la reina.
    ASSERT_TRUE(ApplyFirstMatching(game, IsNonQueenPlacement));
    // Turno 2: las negras colocan cualquier cosa (es su única opción: todavía no
    // tienen nada en el tablero para mover).
    ASSERT_TRUE(ApplyFirstMatching(game, [](const Move&) { return true; }));

    // Turno 3: de nuevo las blancas, que siguen sin colocar la reina. No tiene que
    // aparecer ningún movimiento, aunque tengan una pieza en el tablero.
    ASSERT_FALSE(game.player(Color::White).hasPlacedQueen());
    for (const auto& move : game.legalMoves()) {
        EXPECT_TRUE(std::holds_alternative<Placement>(move));
    }
}

TEST(Game, ApplyingAMovementRelocatesThePieceAndSetsLastMoved) {
    Game game;

    ASSERT_TRUE(ApplyFirstMatching(
        game, [](const Move& move) { return IsPlacementOfType(move, PieceType::Queen); }));
    ASSERT_TRUE(ApplyFirstMatching(game, [](const Move&) { return true; }));

    // Turno 3: ahora la reina de las blancas tiene que tener movimientos.
    Move movementMove;
    bool found = false;
    for (const auto& move : game.legalMoves()) {
        if (std::holds_alternative<Movement>(move)) {
            movementMove = move;
            found = true;
            break;
        }
    }
    ASSERT_TRUE(found);
    const auto movement = std::get<Movement>(movementMove);

    ASSERT_TRUE(game.applyMove(movementMove));

    EXPECT_FALSE(game.board().isOccupied(movement.from));
    Piece* moved = game.board().topAt(movement.destination);
    ASSERT_NE(moved, nullptr);
    EXPECT_EQ(game.lastMoved(), moved);
    EXPECT_EQ(game.turn(), Color::Black);
}

TEST(Game, AnInterveningPlacementReplacesLastMovedFreeingThePreviouslyMovedPiece) {
    Game game;

    ASSERT_TRUE(ApplyFirstMatching(
        game, [](const Move& move) { return IsPlacementOfType(move, PieceType::Queen); }));
    ASSERT_TRUE(ApplyFirstMatching(game, [](const Move&) { return true; }));

    // Turno 3: las blancas mueven la reina.
    Move movementMove;
    for (const auto& move : game.legalMoves()) {
        if (std::holds_alternative<Movement>(move)) {
            movementMove = move;
            break;
        }
    }
    ASSERT_TRUE(game.applyMove(movementMove));
    Piece* movedPiece = game.lastMoved();
    ASSERT_NE(movedPiece, nullptr);
    const Hex movedTo = std::get<Movement>(movementMove).destination;

    // Turno 4: las negras colocan, y la pieza colocada reemplaza a lastMoved_.
    Move placementMove;
    for (const auto& move : game.legalMoves()) {
        if (std::holds_alternative<Placement>(move)) {
            placementMove = move;
            break;
        }
    }
    ASSERT_TRUE(game.applyMove(placementMove));
    Piece* placedPiece = game.board().topAt(std::get<Placement>(placementMove).destination);
    ASSERT_NE(placedPiece, nullptr);

    // Turno 5: otra vez las blancas. lastMoved_ ahora es la pieza negra recién
    // colocada, así que la reina blanca está libre para moverse de nuevo.
    EXPECT_EQ(game.lastMoved(), placedPiece);
    bool queenMovementOffered = false;
    for (const auto& move : game.legalMoves()) {
        if (const auto* m = std::get_if<Movement>(&move)) {
            if (m->from == movedTo) {
                queenMovementOffered = true;
            }
        }
    }
    EXPECT_TRUE(queenMovementOffered);
}

// --- legalMoves() / applyMove(): el lanzamiento del bicho bolita --------

TEST(Game, PillbugThrowRelocatesTheVictimAndSetsLastMovedToIt) {
    Game game;
    const Hex pillbugHex{0, 0};

    // Turno 1: las blancas colocan el bicho bolita (en un tablero vacío el único lugar
    // legal es el origen).
    ASSERT_TRUE(ApplyFirstMatching(
        game, [](const Move& move) { return IsPlacementOfType(move, PieceType::Pillbug); }));
    // Turno 2: las negras colocan cualquier cosa.
    ASSERT_TRUE(ApplyFirstMatching(game, [](const Move&) { return true; }));
    // Turno 3: las blancas colocan la reina. Su única pieza hasta ahora es el bicho
    // bolita, así que todo lugar legal es automáticamente uno de sus vecinos.
    ASSERT_TRUE(ApplyFirstMatching(
        game, [](const Move& move) { return IsPlacementOfType(move, PieceType::Queen); }));
    // Turno 4: las negras colocan cualquier cosa.
    ASSERT_TRUE(ApplyFirstMatching(game, [](const Move&) { return true; }));
    // Turno 5: las blancas colocan una tercera pieza —la futura víctima del
    // lanzamiento— pegada al bicho bolita a propósito.
    ASSERT_TRUE(ApplyFirstMatching(game, [&](const Move& move) {
        const auto* p = std::get_if<Placement>(&move);
        return p && p->type != PieceType::Queen && p->type != PieceType::Pillbug &&
               distance(p->destination, pillbugHex) == 1;
    }));
    // Turno 6: las negras colocan cualquier cosa.
    ASSERT_TRUE(ApplyFirstMatching(game, [](const Move&) { return true; }));

    // Turno 7: es el 4º turno de las blancas, pero la reina ya está colocada, así que
    // no hay restricción. Ahora tiene que haber un PillbugThrow disponible.
    Move throwMove;
    bool found = false;
    for (const auto& move : game.legalMoves()) {
        if (std::holds_alternative<PillbugThrow>(move)) {
            throwMove = move;
            found = true;
            break;
        }
    }
    ASSERT_TRUE(found);
    const auto t = std::get<PillbugThrow>(throwMove);
    EXPECT_TRUE(t.pillbug == pillbugHex);

    Piece* victim = game.board().topAt(t.victim);
    ASSERT_NE(victim, nullptr);

    ASSERT_TRUE(game.applyMove(throwMove));

    EXPECT_FALSE(game.board().isOccupied(t.victim));
    EXPECT_EQ(game.board().topAt(t.destination), victim);
    EXPECT_EQ(game.lastMoved(), victim);  // the victim, not the pillbug
    EXPECT_EQ(game.turn(), Color::Black);
}

// --- legalMoves() / applyMove(): el mosquito ----------------------------
//
// Un mosquito colocado como único vínculo entre dos ramas de la colmena que si no
// quedarían separadas está clavado (Board::canMove), igual que cualquier otra pieza.
// Por eso estos fixtures colocan primero la reina blanca y el mosquito pegado solo a
// ella: así el mosquito queda como hoja, nunca como punto de corte, hasta que una
// colocación posterior lo mete adentro de un ciclo.

TEST(Game, GroundLevelMosquitoOffersImitationMovesAndApplyingOneRelocatesIt) {
    Game game;
    const Hex queenHex{0, 0};

    // Turno 1: las blancas colocan la reina (el único lugar legal es el origen).
    ASSERT_TRUE(ApplyFirstMatching(
        game, [](const Move& move) { return IsPlacementOfType(move, PieceType::Queen); }));
    // Turno 2: las negras colocan su reina (forzosamente pegada a la blanca).
    ASSERT_TRUE(ApplyFirstMatching(
        game, [](const Move& move) { return IsPlacementOfType(move, PieceType::Queen); }));
    // Turno 3: las blancas colocan el mosquito pegado a su propia reina, que es su
    // única pieza hasta ahora, así que cae como hoja (grado 1).
    ASSERT_TRUE(ApplyFirstMatching(game, [&](const Move& move) {
        return IsPlacementOfTypeAdjacentTo(move, PieceType::Mosquito, queenHex);
    }));
    // Turno 4: las negras colocan cualquier cosa.
    ASSERT_TRUE(ApplyFirstMatching(game, [](const Move&) { return true; }));

    Hex mosquitoHex{};
    for (const auto& h : game.board().occupiedHexes()) {
        if (game.board().topAt(h)->type == PieceType::Mosquito) {
            mosquitoHex = h;
        }
    }

    // Turno 5: el mosquito de las blancas es hoja y por lo tanto nunca está clavado,
    // así que tiene que ofrecer MosquitoMovement.
    Move imitationMove;
    bool found = false;
    for (const auto& move : game.legalMoves()) {
        if (const auto* m = std::get_if<MosquitoMovement>(&move)) {
            if (m->from == mosquitoHex) {
                imitationMove = move;
                found = true;
                break;
            }
        }
    }
    ASSERT_TRUE(found);
    const auto chosen = std::get<MosquitoMovement>(imitationMove);

    ASSERT_TRUE(game.applyMove(imitationMove));

    EXPECT_FALSE(game.board().isOccupied(mosquitoHex));
    Piece* mosquito = game.board().topAt(chosen.destination);
    ASSERT_NE(mosquito, nullptr);
    EXPECT_EQ(mosquito->type, PieceType::Mosquito);
    EXPECT_EQ(game.lastMoved(), mosquito);
    EXPECT_EQ(game.turn(), Color::Black);
    EXPECT_EQ(game.turnNumber(), 6);
}

// Un mosquito que toca un bicho bolita recibe todo lo que recibiría un bicho bolita de
// verdad: entradas MosquitoMovement{imitating=Pillbug} —su movimiento propio es
// QueenMovement, el mismo mapeo de StrategyFactory que usa un bicho bolita real— Y
// entradas PillbugThrow, prestándose la habilidad especial. Las dos cosas juntas, igual
// que Game::addMoves() se las ofrece a un bicho bolita de verdad al mismo tiempo.
// Ninguna excluye a la otra, y ninguna depende de que el mosquito esté tocando además
// algún otro tipo.

// Cuando el bicho bolita es el ÚNICO vecino del mosquito, el lanzamiento se intenta
// pero nunca está disponible de verdad: pillbugThrows() pide una víctima que no esté
// clavada (Board::canMove), y el bicho bolita es lo único que conecta al mosquito con
// el resto de la colmena, así que sacarlo aislaría al propio mosquito. En esta
// configuración está siempre clavado. Eso es una consecuencia real y demostrable de la
// regla de la colmena, no un agujero: el test
// MosquitoTouchingPillbugAndAnotherTypeStillOffersTheThrow, más abajo, arma una
// topología (un ciclo de 3) donde el bicho bolita genuinamente no está clavado y el
// lanzamiento sí aparece.
//
// Acá el orden de colocación importa: primero la reina (turno 1), después la pieza
// negra que queda forzosamente pegada (turno 2), y recién entonces el bicho bolita
// (turno 3). Si el bicho bolita fuera primero, sería la única pieza a la que las negras
// pueden pegarse en su primera colocación, y por la geometría del hexágono, un vecino
// colocado contra una pieza sola tapa también las dos direcciones que lo flanquean: al
// turno 4 quedarían abiertas solamente las tres direcciones que flanquean a la reina.
// Elegir cualquiera de esas para la reina deja las otras dos como su propio par de
// flanqueo, o sea que todo lugar que quede pegado al bicho bolita tocaría también a la
// reina, y la premisa de este test sería inalcanzable. Empezando por la reina se
// esquiva la trampa: colocar el bicho bolita en el turno 3 abre vecinos nuevos del otro
// lado, que la única pieza negra anterior no toca.
TEST(Game, MosquitoTouchingOnlyPillbugOffersMovementButThrowIsUnavailable) {
    Game game;

    // Turno 1: las blancas colocan la reina (el único lugar legal es el origen).
    ASSERT_TRUE(ApplyFirstMatching(
        game, [](const Move& move) { return IsPlacementOfType(move, PieceType::Queen); }));
    // Turno 2: las negras colocan cualquier cosa.
    ASSERT_TRUE(ApplyFirstMatching(game, [](const Move&) { return true; }));
    // Turno 3: las blancas colocan el bicho bolita. Su única pieza hasta ahora es la
    // reina, así que cae pegado a ella automáticamente.
    ASSERT_TRUE(ApplyFirstMatching(
        game, [](const Move& move) { return IsPlacementOfType(move, PieceType::Pillbug); }));
    // Turno 4: las negras colocan cualquier cosa.
    ASSERT_TRUE(ApplyFirstMatching(game, [](const Move&) { return true; }));

    Hex queenHex{};
    Hex pillbugHex{};
    for (const auto& h : game.board().occupiedHexes()) {
        if (game.board().topAt(h)->color == Color::White &&
            game.board().topAt(h)->type == PieceType::Queen) {
            queenHex = h;
        }
        if (game.board().topAt(h)->color == Color::White &&
            game.board().topAt(h)->type == PieceType::Pillbug) {
            pillbugHex = h;
        }
    }

    // Turno 5: las blancas colocan el mosquito pegado al bicho bolita pero NO a la
    // reina, así el bicho bolita es su único vecino imitable.
    ASSERT_TRUE(ApplyFirstMatching(game, [&](const Move& move) {
        const auto* p = std::get_if<Placement>(&move);
        return p && p->type == PieceType::Mosquito &&
               distance(p->destination, pillbugHex) == 1 &&
               distance(p->destination, queenHex) != 1;
    }));
    // Turno 6: las negras colocan cualquier cosa.
    ASSERT_TRUE(ApplyFirstMatching(game, [](const Move&) { return true; }));

    // Filtrado por color: el "colocan cualquier cosa" de las negras en el turno 6 puede
    // ser también un mosquito, y una búsqueda sin filtrar podría agarrar ese casillero
    // en vez del de las blancas.
    Hex mosquitoHex{};
    for (const auto& h : game.board().occupiedHexes()) {
        if (game.board().topAt(h)->color == Color::White &&
            game.board().topAt(h)->type == PieceType::Mosquito) {
            mosquitoHex = h;
        }
    }

    bool sawMovement = false;
    bool sawThrow = false;
    for (const auto& move : game.legalMoves()) {
        if (const auto* m = std::get_if<MosquitoMovement>(&move)) {
            if (m->from == mosquitoHex && m->imitating == PieceType::Pillbug) {
                sawMovement = true;
            }
        } else if (const auto* t = std::get_if<PillbugThrow>(&move)) {
            if (t->pillbug == mosquitoHex) {
                sawThrow = true;
            }
        }
    }
    EXPECT_TRUE(sawMovement);
    EXPECT_FALSE(sawThrow);
}

TEST(Game, MosquitoTouchingPillbugAndAnotherTypeStillOffersTheThrow) {
    Game game;
    const Hex pillbugHex{0, 0};

    // Turno 1: las blancas colocan el bicho bolita (el único lugar legal es el origen).
    ASSERT_TRUE(ApplyFirstMatching(
        game, [](const Move& move) { return IsPlacementOfType(move, PieceType::Pillbug); }));
    // Turno 2: las negras colocan cualquier cosa.
    ASSERT_TRUE(ApplyFirstMatching(game, [](const Move&) { return true; }));
    // Turno 3: las blancas colocan la reina. Su única pieza hasta ahora es el bicho
    // bolita, así que cae pegada a él automáticamente.
    ASSERT_TRUE(ApplyFirstMatching(
        game, [](const Move& move) { return IsPlacementOfType(move, PieceType::Queen); }));
    // Turno 4: las negras colocan cualquier cosa.
    ASSERT_TRUE(ApplyFirstMatching(game, [](const Move&) { return true; }));

    Hex queenHex{};
    for (const auto& h : game.board().occupiedHexes()) {
        if (game.board().topAt(h)->color == Color::White &&
            game.board().topAt(h)->type == PieceType::Queen) {
            queenHex = h;
        }
    }

    // Turno 5: las blancas colocan el mosquito pegado al bicho bolita Y a la reina, o
    // sea tocando dos tipos imitables distintos.
    ASSERT_TRUE(ApplyFirstMatching(game, [&](const Move& move) {
        return IsPlacementOfTypeAdjacentToBoth(move, PieceType::Mosquito, pillbugHex, queenHex);
    }));
    // Turno 6: las negras colocan cualquier cosa.
    ASSERT_TRUE(ApplyFirstMatching(game, [](const Move&) { return true; }));

    // Filtrado por color: el "colocan cualquier cosa" de las negras en el turno 6 puede
    // ser también un mosquito, y una búsqueda sin filtrar podría agarrar ese casillero
    // en vez del de las blancas.
    Hex mosquitoHex{};
    for (const auto& h : game.board().occupiedHexes()) {
        if (game.board().topAt(h)->color == Color::White &&
            game.board().topAt(h)->type == PieceType::Mosquito) {
            mosquitoHex = h;
        }
    }

    bool sawQueenImitation = false;
    bool sawThrow = false;
    for (const auto& move : game.legalMoves()) {
        if (const auto* m = std::get_if<MosquitoMovement>(&move)) {
            if (m->from == mosquitoHex && m->imitating == PieceType::Queen) {
                sawQueenImitation = true;
            }
        } else if (const auto* t = std::get_if<PillbugThrow>(&move)) {
            if (t->pillbug == mosquitoHex) {
                sawThrow = true;
            }
        }
    }
    EXPECT_TRUE(sawQueenImitation);
    EXPECT_TRUE(sawThrow);
}

TEST(Game, ElevatedMosquitoPersistsBeetleStrategyAndOffersPlainMovementOnly) {
    Game game;
    const Hex queenHex{0, 0};

    // Turno 1: las blancas colocan la reina (el único lugar legal es el origen).
    ASSERT_TRUE(ApplyFirstMatching(
        game, [](const Move& move) { return IsPlacementOfType(move, PieceType::Queen); }));
    // Turno 2: las negras colocan su reina (forzosamente pegada a la blanca), así más
    // adelante pueden mover: hace falta para sacarle el lastMoved_ de encima al
    // mosquito después de que se suba.
    ASSERT_TRUE(ApplyFirstMatching(
        game, [](const Move& move) { return IsPlacementOfType(move, PieceType::Queen); }));
    // Turno 3: las blancas colocan el mosquito pegado a su propia reina, que es su
    // única pieza hasta ahora, así que cae como hoja (grado 1).
    ASSERT_TRUE(ApplyFirstMatching(game, [&](const Move& move) {
        return IsPlacementOfTypeAdjacentTo(move, PieceType::Mosquito, queenHex);
    }));
    // Turno 4: las negras colocan cualquier cosa.
    ASSERT_TRUE(ApplyFirstMatching(game, [](const Move&) { return true; }));

    Hex mosquitoHex{};
    for (const auto& h : game.board().occupiedHexes()) {
        if (game.board().topAt(h)->type == PieceType::Mosquito) {
            mosquitoHex = h;
        }
    }

    // Turno 5: las blancas colocan un escarabajo pegado a la reina Y al mosquito, con lo
    // que el mosquito queda adentro de un ciclo (reina - mosquito - escarabajo - reina) y
    // sigue pudiendo moverse, en vez de convertirse en el punto de corte entre dos hojas
    // que si no quedarían separadas.
    ASSERT_TRUE(ApplyFirstMatching(game, [&](const Move& move) {
        return IsPlacementOfTypeAdjacentToBoth(move, PieceType::Beetle, mosquitoHex, queenHex);
    }));
    // Turno 6: las negras colocan cualquier cosa.
    ASSERT_TRUE(ApplyFirstMatching(game, [](const Move&) { return true; }));

    // Turno 7: el mosquito de las blancas imita al escarabajo y se sube al casillero de
    // algún vecino ocupado.
    Move climbMove;
    bool foundClimb = false;
    for (const auto& move : game.legalMoves()) {
        const auto* m = std::get_if<MosquitoMovement>(&move);
        if (m && m->imitating == PieceType::Beetle && game.board().isOccupied(m->destination)) {
            climbMove = move;
            foundClimb = true;
            break;
        }
    }
    ASSERT_TRUE(foundClimb);
    const Hex elevatedHex = std::get<MosquitoMovement>(climbMove).destination;
    ASSERT_TRUE(game.applyMove(climbMove));
    ASSERT_EQ(game.board().stackHeight(elevatedHex), 2u);

    // Turno 8: las negras mueven (no colocan), así lastMoved_ deja de apuntar al
    // mosquito. Una colocación no lo limpiaría.
    ASSERT_TRUE(ApplyFirstMatching(
        game, [](const Move& move) { return std::holds_alternative<Movement>(move); }));

    // Turno 9: el Piece::movement del mosquito en altura ya tiene que ser una
    // BeetleMovement, que se le dejó fija cuando subió, y legalMoves() tiene que ofrecer
    // un Movement común desde su casillero, nunca una elección nueva de a quién imitar.
    Piece* mosquito = game.board().topAt(elevatedHex);
    ASSERT_NE(mosquito, nullptr);
    EXPECT_EQ(mosquito->type, PieceType::Mosquito);
    ASSERT_NE(mosquito->movement, nullptr);
    EXPECT_NE(dynamic_cast<BeetleMovement*>(mosquito->movement.get()), nullptr);

    bool sawMovementFromMosquito = false;
    for (const auto& move : game.legalMoves()) {
        if (const auto* m = std::get_if<Movement>(&move)) {
            if (m->from == elevatedHex) {
                sawMovementFromMosquito = true;
            }
        }
        if (const auto* mm = std::get_if<MosquitoMovement>(&move)) {
            EXPECT_FALSE(mm->from == elevatedHex)
                << "elevated mosquito must not offer a fresh imitation choice";
        }
    }
    EXPECT_TRUE(sawMovementFromMosquito);
}

TEST(Game, ElevatedMosquitoRevertsToFreshImitationAfterDescending) {
    Game game;
    const Hex queenHex{0, 0};

    // Turnos 1 a 7: el mismo armado que el test de arriba, el de la estrategia que
    // persiste en altura: subir el mosquito a un escarabajo vecino.
    ASSERT_TRUE(ApplyFirstMatching(
        game, [](const Move& move) { return IsPlacementOfType(move, PieceType::Queen); }));
    ASSERT_TRUE(ApplyFirstMatching(
        game, [](const Move& move) { return IsPlacementOfType(move, PieceType::Queen); }));
    ASSERT_TRUE(ApplyFirstMatching(game, [&](const Move& move) {
        return IsPlacementOfTypeAdjacentTo(move, PieceType::Mosquito, queenHex);
    }));
    ASSERT_TRUE(ApplyFirstMatching(game, [](const Move&) { return true; }));

    Hex mosquitoHex{};
    for (const auto& h : game.board().occupiedHexes()) {
        if (game.board().topAt(h)->type == PieceType::Mosquito) {
            mosquitoHex = h;
        }
    }

    ASSERT_TRUE(ApplyFirstMatching(game, [&](const Move& move) {
        return IsPlacementOfTypeAdjacentToBoth(move, PieceType::Beetle, mosquitoHex, queenHex);
    }));
    ASSERT_TRUE(ApplyFirstMatching(game, [](const Move&) { return true; }));

    Move climbMove;
    bool foundClimb = false;
    for (const auto& move : game.legalMoves()) {
        const auto* m = std::get_if<MosquitoMovement>(&move);
        if (m && m->imitating == PieceType::Beetle && game.board().isOccupied(m->destination)) {
            climbMove = move;
            foundClimb = true;
            break;
        }
    }
    ASSERT_TRUE(foundClimb);
    const Hex elevatedHex = std::get<MosquitoMovement>(climbMove).destination;
    ASSERT_TRUE(game.applyMove(climbMove));
    ASSERT_EQ(game.board().stackHeight(elevatedHex), 2u);

    // Le sacamos el lastMoved_ de encima al mosquito, así vuelve a estar habilitado para
    // actuar.
    ASSERT_TRUE(ApplyFirstMatching(
        game, [](const Move& move) { return std::holds_alternative<Movement>(move); }));

    Piece* mosquito = game.board().topAt(elevatedHex);
    ASSERT_NE(mosquito, nullptr);
    ASSERT_NE(dynamic_cast<BeetleMovement*>(mosquito->movement.get()), nullptr);

    // Bajar: aplicamos el Movement común que deje al mosquito de vuelta en un casillero
    // vacío. El moves() de BeetleMovement lo permite estando en altura, igual que un
    // escarabajo de verdad bajándose de una pila.
    Move descendMove;
    bool foundDescend = false;
    for (const auto& move : game.legalMoves()) {
        const auto* m = std::get_if<Movement>(&move);
        if (m && m->from == elevatedHex && !game.board().isOccupied(m->destination)) {
            descendMove = move;
            foundDescend = true;
            break;
        }
    }
    ASSERT_TRUE(foundDescend);
    const Hex groundHex = std::get<Movement>(descendMove).destination;
    ASSERT_TRUE(game.applyMove(descendMove));
    ASSERT_EQ(game.board().stackHeight(groundHex), 1u);
    EXPECT_EQ(mosquito->movement, nullptr);

    // Le sacamos otra vez el lastMoved_ de encima al mosquito. Limitado a destinos que
    // no sean vecinos de groundHex: si las negras reubican una pieza al lado del
    // mosquito que acaba de bajar, esa pieza podría quedar colgada del mosquito como
    // único puente a la colmena, y entonces quedaría clavada justo la pieza que este
    // test necesita mover el turno siguiente. Sería un accidente de qué movida vino
    // primero, sin relación con lo que se está probando acá.
    ASSERT_TRUE(ApplyFirstMatching(game, [&](const Move& move) {
        const auto* m = std::get_if<Movement>(&move);
        return m && distance(m->destination, groundHex) != 1;
    }));

    // De vuelta a nivel del piso, tiene que volver a ofrecer una elección nueva de a
    // quién imitar, no la estrategia de escarabajo vieja, que ya se revirtió.
    bool sawImitation = false;
    for (const auto& move : game.legalMoves()) {
        if (const auto* mm = std::get_if<MosquitoMovement>(&move)) {
            if (mm->from == groundHex) {
                sawImitation = true;
            }
        }
    }
    EXPECT_TRUE(sawImitation);
}

// --- detección de victoria y empate --------------------------------------
//
// Rodear una reina por completo haciendo solamente movidas legales lleva una secuencia
// de varios turnos de verdad. Estos tests amontonan piezas alrededor del casillero
// objetivo con lo que legalMoves() vaya ofreciendo, en vez de hardcodear coordenadas,
// así no se rompen si cambia el orden interno en que se recorren las movidas.

TEST(Game, SurroundingAQueenEndsTheGameForTheOpponentToWin) {
    Game game;
    const Hex queenHex{0, 0};

    ASSERT_TRUE(ApplyFirstMatching(
        game, [](const Move& move) { return IsPlacementOfType(move, PieceType::Queen); }));

    int guard = 0;
    while (game.status() == GameStatus::Ongoing && ++guard <= 60) {
        if (ApplyFirstMatching(
                game, [&](const Move& move) { return IsNonQueenPlacementAdjacentTo(move, queenHex); })) {
            continue;
        }
        if (ApplyFirstMatching(
                game, [&](const Move& move) { return IsMovementAdjacentTo(move, queenHex); })) {
            continue;
        }
        ASSERT_TRUE(ApplyFirstMatching(game, [](const Move&) { return true; }));
    }

    ASSERT_TRUE(game.board().isSurrounded(queenHex));
    EXPECT_EQ(game.status(), GameStatus::BlackWins);
    EXPECT_EQ(game.winner(), Color::Black);
    EXPECT_TRUE(game.legalMoves().empty());
}

TEST(Game, SimultaneouslySurroundingBothQueensIsADraw) {
    Game game;

    ASSERT_TRUE(ApplyFirstMatching(
        game, [](const Move& move) { return IsPlacementOfType(move, PieceType::Queen); }));
    ASSERT_TRUE(ApplyFirstMatching(
        game, [](const Move& move) { return IsPlacementOfType(move, PieceType::Queen); }));

    const Hex queenA{0, 0};
    Hex queenB{};
    for (const auto& h : game.board().occupiedHexes()) {
        if (!(h == queenA)) {
            queenB = h;
        }
    }

    // Todo par de casilleros vecinos comparte exactamente 2 vecinos. Esos 2 (`shared`)
    // son los únicos a los que una colocación nueva nunca puede llegar legalmente,
    // porque tocan las dos reinas a la vez: para el color que lo intente, una de ellas
    // es siempre "la reina rival". Solo se pueden llenar después, moviendo. Si primero
    // se llenan colocando todos los demás vecinos de las dos reinas, y recién al final
    // se cae en los 2 casilleros compartidos moviendo, la última movida cierra los dos
    // anillos de golpe.
    std::vector<Hex> aOnly, bOnly, shared;
    for (const Hex& h : neighbors(queenA)) {
        if (h == queenB) continue;
        (distance(h, queenB) == 1 ? shared : aOnly).push_back(h);
    }
    for (const Hex& h : neighbors(queenB)) {
        if (h == queenA || distance(h, queenA) == 1) continue;
        bOnly.push_back(h);
    }
    ASSERT_EQ(shared.size(), 2u);
    ASSERT_EQ(aOnly.size(), 3u);
    ASSERT_EQ(bOnly.size(), 3u);
    const Hex f1 = shared[0];
    const Hex f2 = shared[1];

    for (std::size_t i = 0; i < aOnly.size(); ++i) {
        ASSERT_TRUE(ApplyFirstMatching(
            game, [&](const Move& move) { return IsNonQueenPlacementAt(move, aOnly[i]); }));
        ASSERT_TRUE(ApplyFirstMatching(
            game, [&](const Move& move) { return IsNonQueenPlacementAt(move, bOnly[i]); }));
    }

    std::vector<Hex> ring = {queenA, queenB};
    ring.insert(ring.end(), aOnly.begin(), aOnly.end());
    ring.insert(ring.end(), bOnly.begin(), bOnly.end());

    bool filledF2 = false;
    int guard = 0;
    while (game.status() == GameStatus::Ongoing && ++guard <= 60) {
        const Hex& target = filledF2 ? f1 : f2;
        const bool movedIn = ApplyFirstMatching(game, [&](const Move& move) {
            const auto* m = std::get_if<Movement>(&move);
            return m && m->destination == target && !Contains(ring, m->from);
        });
        if (movedIn) {
            if (!filledF2) {
                filledF2 = true;
            }
            continue;
        }
        // Relleno: colocamos una pieza de sobra cerca de f1/f2 —nunca sobre el anillo ni
        // sobre f1/f2 mismos— para que después haya algo con lo que entrar moviendo.
        const bool placedFiller = ApplyFirstMatching(game, [&](const Move& move) {
            const auto* p = std::get_if<Placement>(&move);
            if (!p || Contains(ring, p->destination) || p->destination == f1 ||
                p->destination == f2) {
                return false;
            }
            return distance(p->destination, f1) == 1 || distance(p->destination, f2) == 1;
        });
        if (placedFiller) continue;
        // Si no, cualquier movida legal que no saque una pieza del anillo de su puesto.
        ASSERT_TRUE(ApplyFirstMatching(game, [&](const Move& move) {
            const auto* m = std::get_if<Movement>(&move);
            return !(m && Contains(ring, m->from));
        }));
    }

    ASSERT_TRUE(game.board().isSurrounded(queenA));
    ASSERT_TRUE(game.board().isSurrounded(queenB));
    EXPECT_EQ(game.status(), GameStatus::Draw);
    EXPECT_EQ(game.winner(), std::nullopt);
    EXPECT_TRUE(game.legalMoves().empty());
}

// --- de quién son las piezas, y cuál puede actuar ------------------------

namespace {

// Busca el casillero de la (única) pieza de `color` y `type`, o un Hex por defecto si no
// hay ninguna.
Hex FindPiece(const Game& game, Color color, PieceType type) {
    for (const auto& h : game.board().occupiedHexes()) {
        const Piece* p = game.board().topAt(h);
        if (p->color == color && p->type == type) {
            return h;
        }
    }
    return Hex{99, 99};
}

// Si `move` movería una pieza que no es de `color`, o usaría una habilidad desde el
// casillero de una pieza así.
bool ActsOnAPieceNotOwnedBy(const Game& game, const Move& move, Color color) {
    Hex origin{99, 99};
    if (const auto* m = std::get_if<Movement>(&move)) {
        origin = m->from;
    } else if (const auto* mm = std::get_if<MosquitoMovement>(&move)) {
        origin = mm->from;
    } else if (const auto* t = std::get_if<PillbugThrow>(&move)) {
        origin = t->pillbug;  // the thrower, not the victim: a pillbug may
                              // puede lanzar legalmente piezas de cualquier color
    } else {
        return false;  // Placement: comes from the hand, not the board
    }
    const Piece* p = game.board().topAt(origin);
    return p != nullptr && p->color != color;
}

// Bicho bolita blanco en el origen, con una reina negra y una hormiga negra tocándolo.
// La hormiga tiene que llegar al contacto MOVIÉNDOSE: colocar una pieza negra tocando una
// blanca no es legal. Y ese segundo contacto entre negras y blancas es exactamente lo
// que desclava a la reina negra: sin él, todo el grupo negro cuelga de la reina, que
// entonces es un punto de corte y nunca puede ser víctima de un lanzamiento.
bool BuildEnemyThrowPosition(Game& game, Hex& pillbugHex, Hex& blackQueenHex,
                              Hex& blackAntHex) {
    pillbugHex = Hex{0, 0};
    auto placement = [](PieceType t) {
        return [t](const Move& m) { return IsPlacementOfType(m, t); };
    };
    if (!ApplyFirstMatching(game, placement(PieceType::Pillbug))) return false;  // W1
    if (!ApplyFirstMatching(game, placement(PieceType::Queen))) return false;    // B2
    if (!ApplyFirstMatching(game, placement(PieceType::Queen))) return false;    // W3
    if (!ApplyFirstMatching(game, placement(PieceType::Ant))) return false;      // B4
    if (!ApplyFirstMatching(game, [](const Move& m) {                            // W5 filler
            return std::holds_alternative<Placement>(m);
        })) return false;

    blackQueenHex = FindPiece(game, Color::Black, PieceType::Queen);
    blackAntHex = FindPiece(game, Color::Black, PieceType::Ant);

    // B6: la hormiga camina hasta tocar al bicho bolita.
    if (!ApplyFirstMatching(game, [&](const Move& m) {
            const auto* mv = std::get_if<Movement>(&m);
            return mv && mv->from == blackAntHex &&
                   distance(mv->destination, pillbugHex) == 1;
        })) return false;
    blackAntHex = FindPiece(game, Color::Black, PieceType::Ant);
    return true;
}

}  // namespace

TEST(Game, NeverOffersMovesOfTheOpponentsPieces) {
    Game game;

    // Las dos reinas colocadas y después una pieza negra a cada lado de la reina negra,
    // para que en el turno de las blancas haya al menos una pieza negra habilitada en
    // todo sentido menos el color: no clavada (es hoja) y distinta de lastMoved_ (que es
    // la pieza colocada el turno anterior al último).
    ASSERT_TRUE(ApplyFirstMatching(
        game, [](const Move& move) { return IsPlacementOfType(move, PieceType::Queen); }));
    ASSERT_TRUE(ApplyFirstMatching(
        game, [](const Move& move) { return IsPlacementOfType(move, PieceType::Queen); }));
    const Hex blackQueenHex = FindPiece(game, Color::Black, PieceType::Queen);
    ASSERT_TRUE(ApplyFirstMatching(game, IsNonQueenPlacement));                     // W3
    ASSERT_TRUE(ApplyFirstMatching(game, [&](const Move& move) {                    // B4
        return IsNonQueenPlacementAdjacentTo(move, blackQueenHex);
    }));
    const Hex firstBlackLeaf = FindPiece(game, Color::Black, PieceType::Ant);
    ASSERT_TRUE(ApplyFirstMatching(game, IsNonQueenPlacement));                     // W5
    // Una segunda hoja negra del otro lado de la reina, no al lado de la primera, así
    // ninguna de las dos es punto de corte de la otra.
    ASSERT_TRUE(ApplyFirstMatching(game, [&](const Move& move) {                    // B6
        const auto* p = std::get_if<Placement>(&move);
        return p && p->type != PieceType::Queen &&
               distance(p->destination, blackQueenHex) == 1 &&
               distance(p->destination, firstBlackLeaf) != 1;
    }));

    ASSERT_EQ(game.turn(), Color::White);
    ASSERT_TRUE(game.player(Color::White).hasPlacedQueen());

    // Que no sea un test vacío: hay realmente una pieza negra que no está clavada y que
    // no es la que actuó el turno pasado, así que lo único que impide que las blancas la
    // muevan es el color.
    bool eligibleEnemyExists = false;
    for (const auto& h : game.board().occupiedHexes()) {
        Piece* p = game.board().topAt(h);
        if (p->color == Color::Black && p != game.lastMoved() && game.board().canMove(h) &&
            !p->legalMoves(game.board(), h).empty()) {
            eligibleEnemyExists = true;
        }
    }
    ASSERT_TRUE(eligibleEnemyExists);

    for (const auto& move : game.legalMoves()) {
        EXPECT_FALSE(ActsOnAPieceNotOwnedBy(game, move, Color::White))
            << "White was offered a move of a Black piece";
    }
}

TEST(Game, APieceThrownByThePillbugCannotMoveOnItsOwnersNextTurn) {
    // La única situación en la que lastMoved_ se puede observar: normalmente apunta a una
    // pieza del jugador que acaba de mover, al que el filtro de color ya excluye. El
    // lanzamiento es la excepción, porque pone en lastMoved_ a la VÍCTIMA, que suele ser
    // del rival, y esa pieza tiene que perderse el próximo turno de su dueño.
    Game game;
    Hex pillbugHex{}, blackQueenHex{}, blackAntHex{};
    ASSERT_TRUE(BuildEnemyThrowPosition(game, pillbugHex, blackQueenHex, blackAntHex));

    ASSERT_EQ(game.turn(), Color::White);
    ASSERT_TRUE(ApplyFirstMatching(game, [&](const Move& move) {
        const auto* t = std::get_if<PillbugThrow>(&move);
        return t && t->pillbug == pillbugHex && t->victim == blackQueenHex;
    })) << "the black queen should be throwable: adjacent, alone, unpinned, not lastMoved";

    const Hex landedOn = FindPiece(game, Color::Black, PieceType::Queen);
    Piece* victim = game.board().topAt(landedOn);
    ASSERT_EQ(game.lastMoved(), victim);
    ASSERT_EQ(game.turn(), Color::Black);

    // Que no sea un test vacío: la reina lanzada está libre en todo lo demás.
    ASSERT_TRUE(game.board().canMove(landedOn));
    ASSERT_FALSE(victim->legalMoves(game.board(), landedOn).empty());

    for (const auto& move : game.legalMoves()) {
        const auto* m = std::get_if<Movement>(&move);
        EXPECT_FALSE(m && m->from == landedOn)
            << "the piece thrown last turn must sit this turn out";
    }
}

// --- la validación de applyMove() ---------------------------------------
//
// applyMove() decide si una movida es legal buscándola en legalMoves() con std::find, o
// sea a través de operator==. Estos dos tests cubren los campos que no chequea nada más
// que esa búsqueda.

TEST(Game, ApplyMoveRejectsAPlacementOfAnotherTypeOnTheForcedQueenTurn) {
    Game game;
    for (int i = 0; i < 6; ++i) {
        ASSERT_TRUE(ApplyFirstMatching(game, IsNonQueenPlacement));
    }
    ASSERT_EQ(game.turnNumber(), 7);
    ASSERT_FALSE(game.player(Color::White).hasPlacedQueen());

    const auto moves = game.legalMoves();
    ASSERT_FALSE(moves.empty());
    const Hex queenSpot = std::get<Placement>(moves.front()).destination;
    const int antsBefore = game.player(Color::White).remaining(PieceType::Ant);

    EXPECT_FALSE(game.applyMove(Placement{PieceType::Ant, queenSpot}))
        << "same destination, wrong type: still not a legal move";

    EXPECT_FALSE(game.board().isOccupied(queenSpot));
    EXPECT_EQ(game.player(Color::White).remaining(PieceType::Ant), antsBefore);
    EXPECT_EQ(game.turn(), Color::White);
    EXPECT_EQ(game.turnNumber(), 7);
}

TEST(Game, ApplyMoveRejectsAThrowNamingAnIneligibleVictim) {
    Game game;
    Hex pillbugHex{}, blackQueenHex{}, blackAntHex{};
    ASSERT_TRUE(BuildEnemyThrowPosition(game, pillbugHex, blackQueenHex, blackAntHex));

    // La hormiga está pegada al bicho bolita, pero se movió el turno pasado, así que no
    // hay ningún lanzamiento de ella en oferta; el de la reina negra sí.
    Hex destination{99, 99};
    for (const auto& move : game.legalMoves()) {
        const auto* t = std::get_if<PillbugThrow>(&move);
        ASSERT_FALSE(t && t->victim == blackAntHex) << "premise: the ant is not throwable";
        if (t && t->victim == blackQueenHex) {
            destination = t->destination;
        }
    }
    ASSERT_FALSE(destination == (Hex{99, 99})) << "premise: the black queen is throwable";

    EXPECT_FALSE(game.applyMove(PillbugThrow{pillbugHex, blackAntHex, destination}))
        << "legal thrower and destination, ineligible victim: still not a legal move";

    EXPECT_TRUE(game.board().isOccupied(blackAntHex));
    EXPECT_FALSE(game.board().isOccupied(destination));
    EXPECT_EQ(game.turn(), Color::White);
}
