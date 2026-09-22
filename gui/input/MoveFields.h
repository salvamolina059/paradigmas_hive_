#pragma once

#include <optional>

#include "hive/Hex.h"
#include "hive/Move.h"

// The hex a Move was made "from": Movement.from, MosquitoMovement.from,
// or PillbugThrow.pillbug (the thrower, never the victim) -- or
// nullopt for a Placement, which has no source hex (it comes from
// hand, not the board). Used to filter Game::legalMoves() down to
// "everything sourced from this selected hex," the one operation every
// InputState dealing in raw Move lists needs (Idle, ActionMenu).
std::optional<Hex> sourceHex(const Move &move);

// The hex a Move lands on. Every alternative has a .destination field
// with this exact meaning, so this just unwraps the variant instead of
// every caller repeating the same 4-way visit.
Hex destinationHex(const Move &move);
