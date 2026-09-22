#pragma once

#include <optional>
#include <vector>

#include "InputState.h"
#include "hive/PillbugAbility.h"

// A throw's victim is being picked. Reached only from
// ActionMenu::chooseAction(Throw); candidates_ is every PillbugThrow
// from the same thrower (a real Pillbug, or a Mosquito currently
// imitating one).
//
// Deliberately stricter than AwaitingDestination: a stray click here
// is a no-op rather than an implicit cancel (see
// requiresExplicitCancel()) -- a throw is a two-step commitment
// (victim, then destination) that affects a second piece, not just the
// selected one, so it shouldn't be as easy to fumble out of
// accidentally as a plain move.
class AwaitingVictim : public InputState
{
public:
    // `imitating` is carried only for currentImitation()'s sake (the
    // on-board icon persisting through this step) -- it plays no part
    // in picking a victim, since candidates_ is already narrowed to
    // this exact thrower. The core itself never sees or stores a
    // "current imitation": Piece::movement is only ever swapped once a
    // move is actually applied (Game::syncMosquitoStrategy). This is
    // purely leftover UI state for rendering, same idea as
    // selectedHex()/highlightedHexes().
    AwaitingVictim(std::vector<PillbugThrow> candidates, std::optional<PieceType> imitating = std::nullopt);

    // A click matching one candidate's .victim narrows to
    // AwaitingThrowDestination over just that victim's destinations.
    // Anything else: no-op (nullptr) -- only a highlighted victim or
    // the explicit Cancel button (cancel()) leave this state.
    std::unique_ptr<InputState> handleBoardClick(Game &game, const Hex &hex) override;

    // Rebuilds the ActionMenu this thrower's menu would show --
    // candidates_.front().pillbug is the thrower's hex (shared by
    // every candidate).
    std::unique_ptr<InputState> cancel(Game &game) override;

    std::optional<Hex> selectedHex() const override;
    std::vector<Hex> highlightedHexes() const override;
    std::optional<PieceType> currentImitation() const override;
    bool requiresExplicitCancel() const override;

private:
    std::vector<PillbugThrow> candidates_;
    std::optional<PieceType> imitating_;
};
