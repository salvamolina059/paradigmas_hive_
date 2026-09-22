#pragma once

#include <optional>
#include <vector>

#include "InputState.h"
#include "hive/PillbugAbility.h"

// The final step of a throw: a destination for the victim chosen in
// AwaitingVictim. candidates_ is that same thrower's PillbugThrows
// narrowed to just the chosen victim, so they now differ only in
// .destination.
class AwaitingThrowDestination : public InputState
{
public:
    // `imitating`, as in AwaitingVictim, is carried only so
    // currentImitation() can keep the on-board icon up through this
    // last step -- candidates_ alone is enough to pick and apply a
    // destination.
    AwaitingThrowDestination(std::vector<PillbugThrow> candidates, std::optional<PieceType> imitating = std::nullopt);

    // A click matching one candidate's .destination applies that exact
    // PillbugThrow (game.applyMove) and returns a fresh Idle. Anything
    // else: no-op, same reasoning as AwaitingVictim -- only a
    // highlighted destination or the explicit Cancel button leave this
    // state.
    std::unique_ptr<InputState> handleBoardClick(Game &game, const Hex &hex) override;

    // Rebuilds the AwaitingVictim this destination choice narrowed
    // from, by reconstructing the thrower's ActionMenu and asking it
    // for the Throw action again -- reuses that filtering logic rather
    // than re-deriving "every PillbugThrow for this thrower" a second
    // time here.
    std::unique_ptr<InputState> cancel(Game &game) override;

    std::optional<Hex> selectedHex() const override;
    std::vector<Hex> highlightedHexes() const override;
    std::optional<PieceType> currentImitation() const override;
    bool requiresExplicitCancel() const override;

private:
    std::vector<PillbugThrow> candidates_;
    std::optional<PieceType> imitating_;
};
