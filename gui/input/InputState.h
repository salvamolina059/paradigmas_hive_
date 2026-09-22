#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "hive/Enums.h"
#include "hive/Hex.h"

class Game;

// Which of the two action kinds a selected piece's top-bar menu button
// picks. Never Placement -- hand clicks skip the menu entirely and go
// straight to AwaitingDestination (see Idle::handleHandClick):
// placement never had more than one kind of action to disambiguate in
// the first place, so routing it through the menu too would just be
// an extra click for no reason.
enum class ActionKind
{
    Move,
    Throw,
};

// Base of the InputController state hierarchy (mirrors
// MovementStrategy: one class per state -- see Idle.h, ActionMenu.h,
// AwaitingDestination.h, AwaitingVictim.h, AwaitingThrowDestination.h).
// InputController owns a single unique_ptr<InputState> and forwards
// every click/button/query to it.
//
// Every handle*/choose*/cancel method returns the state to transition
// to, or nullptr to mean "no transition -- stay right here" (used both
// for clicks that don't mean anything in the current state, and for
// chooseImitation(), which mutates in place rather than transitioning
// -- see ActionMenu). That's why they're non-pure with a default no-op
// body here rather than pure virtual: most concrete states only care
// about one or two of these events, and repeating an empty override in
// every one of them would just be noise.
//
// There is deliberately no stored "return to" pointer: keeping a
// previous InputState instance alive so it could be handed back later
// would need a polymorphic clone() (copying an InputState isn't
// otherwise meaningful). Instead, each concrete state's cancel()
// override reconstructs its logical predecessor from `game` plus
// whatever minimal fields it already stores for its own purposes --
// the same "derive it, don't cache it" approach Game::legalMoves()
// itself is built on. Idle's default (returns nullptr) needs no
// override, since there's nothing before it to reconstruct.
class InputState
{
public:
    virtual ~InputState() = default;

    virtual std::unique_ptr<InputState> handleBoardClick(Game &game, const Hex &hex);
    virtual std::unique_ptr<InputState> handleHandClick(Game &game, Color color, PieceType type);
    virtual std::unique_ptr<InputState> chooseAction(Game &game, ActionKind kind);

    // Ground-level Mosquito only: pick, or freely switch, which
    // neighbor's type it's currently imitating -- meaningful "at any
    // time" a Mosquito is selected, not gated to one particular click,
    // which is why it's its own method rather than folded into
    // chooseAction. Every other state ignores it (default no-op).
    virtual std::unique_ptr<InputState> chooseImitation(Game &game, PieceType type);

    // Steps back to whatever state this one was logically entered
    // from. Default no-op (returns nullptr), which is exactly right
    // for Idle -- there's nothing before it.
    virtual std::unique_ptr<InputState> cancel(Game &game);

    // Render-facing queries -- default to "nothing", overridden by
    // whichever concrete states have something to show.
    virtual std::optional<Hex> selectedHex() const;

    // The hand piece currently being placed, i.e. AwaitingDestination
    // reached from a hand click rather than a board selection --
    // nullopt everywhere else, including AwaitingDestination for a
    // board-originated Movement/MosquitoMovement (selectedHex() covers
    // that case instead). The two are mutually exclusive: a hand
    // placement has no board hex to ring, and vice versa.
    virtual std::optional<PieceType> selectedHandPieceType() const;

    virtual std::vector<Hex> highlightedHexes() const;
    virtual std::vector<ActionKind> actionOptions() const;
    virtual std::vector<PieceType> imitationOptions() const;
    virtual std::optional<PieceType> currentImitation() const;

    // True only for AwaitingVictim/AwaitingThrowDestination: the
    // pillbug-throw flow needs an explicit, deliberate way out rather
    // than being trivially dismissed by an unrelated click the way a
    // plain move selection can be (see those classes' comments).
    virtual bool requiresExplicitCancel() const;
};
