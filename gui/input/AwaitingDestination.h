#pragma once

#include <optional>
#include <vector>

#include "InputState.h"
#include "hive/Move.h"

// A destination hex is being picked to complete a Placement, Movement,
// or MosquitoMovement (never PillbugThrow -- that's
// AwaitingVictim/AwaitingThrowDestination's job). candidates_ must all
// be the same Move alternative and agree on everything except
// .destination -- every alternative used here has that field, so
// highlighting/matching is written once, generically, instead of once
// per alternative.
class AwaitingDestination : public InputState
{
public:
    // `imitating` is carried only for currentImitation()'s sake (the
    // on-board icon persisting through this step) -- it plays no part
    // in matching a click, since candidates_ is already narrowed to it.
    AwaitingDestination(std::vector<Move> candidates, std::optional<PieceType> imitating = std::nullopt);

    // A click matching one candidate's .destination applies that exact
    // Move (game.applyMove) and returns a fresh Idle. Anything else
    // deselects (-> cancel()) -- a plain destination pick is low-stakes
    // enough that click-elsewhere-cancels is fine (contrast
    // AwaitingVictim/AwaitingThrowDestination, which require the
    // explicit Cancel button instead).
    std::unique_ptr<InputState> handleBoardClick(Game &game, const Hex &hex) override;

    // Lets picking a different hand piece switch the in-progress
    // Placement directly, the same way ActionMenu::handleBoardClick lets
    // clicking a different board piece replace the current selection
    // without an explicit cancel first -- delegates to Idle's own
    // handling of the click (see Idle.h) since "start placing this
    // type" means exactly the same thing whether reached from Idle or
    // from here.
    std::unique_ptr<InputState> handleHandClick(Game &game, Color color, PieceType type) override;

    // Rebuilds an ActionMenu for candidates_'s shared source hex (see
    // selectedHex()) if there is one, or a fresh Idle for a
    // hand-originated Placement, which never had a menu to return to.
    std::unique_ptr<InputState> cancel(Game &game) override;

    // The shared source hex of every Movement/MosquitoMovement
    // candidate, or nullopt for a hand-originated Placement (there's
    // no board hex selected in that case).
    std::optional<Hex> selectedHex() const override;

    // The type being placed, for a hand-originated Placement -- nullopt
    // for a board-originated Movement/MosquitoMovement (selectedHex()
    // covers that case instead; see InputState.h).
    std::optional<PieceType> selectedHandPieceType() const override;

    std::vector<Hex> highlightedHexes() const override;
    std::optional<PieceType> currentImitation() const override;

private:
    std::vector<Move> candidates_;
    std::optional<PieceType> imitating_;
};
