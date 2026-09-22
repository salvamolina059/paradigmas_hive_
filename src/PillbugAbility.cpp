#include "hive/PillbugAbility.h"

std::vector<PillbugThrow> pillbugThrows(const Board &board, const Hex &at,
                                         const Piece *lastMoved)
{
    std::vector<Hex> victims;
    for (const Hex &neighbor : board.occupiedNeighbors(at))
    {
        if (board.stackHeight(neighbor) != 1)
        {
            continue; // stacks are never movable by a throw
        }
        if (board.topAt(neighbor) == lastMoved)
        {
            continue;
        }
        if (!board.canMove(neighbor))
        {
            continue; // pinned
        }
        victims.push_back(neighbor);
    }

    const std::vector<Hex> destinations = board.emptyNeighbors(at);

    std::vector<PillbugThrow> result;
    for (const Hex &victim : victims)
    {
        for (const Hex &destination : destinations)
        {
            result.push_back(PillbugThrow{at, victim, destination});
        }
    }
    return result;
}
