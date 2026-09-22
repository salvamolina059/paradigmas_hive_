#include "hive/movement/SpiderMovement.h"

#include "hive/Board.h"

#include <unordered_set>

namespace {

// DFS con backtracking sobre caminos simples, sin repetir casilleros: `visited` es
// del camino (se desmarca al volver), no un conjunto global, porque dos ramas
// distintas pueden pasar legítimamente por el mismo casillero. El tablero nunca se
// modifica: que `self` esté levantada durante todo el recorrido se consigue con
// wouldStayAttached, no moviéndola de verdad.
void collectDestinationsAtDepthThree(const Board &board, const Piece &self,
                                      const Hex &cur, int depth,
                                      std::unordered_set<Hex> &visited,
                                      std::unordered_set<Hex> &destinations)
{
    if (depth == 3)
    {
        destinations.insert(cur);
        return;
    }

    for (const Hex &next : neighbors(cur))
    {
        if (visited.count(next) > 0)
        {
            continue; // no backtracking within this path
        }
        if (board.isOccupied(next))
        {
            continue;
        }
        if (!board.canSlide(cur, next))
        {
            continue;
        }
        if (!board.wouldStayAttached(next, self))
        {
            continue;
        }

        visited.insert(next);
        collectDestinationsAtDepthThree(board, self, next, depth + 1, visited, destinations);
        visited.erase(next);
    }
}

} // namespace

std::vector<Hex> SpiderMovement::moves(const Board &board, const Hex &from,
                                        const Piece &self) const
{
    std::unordered_set<Hex> visited{from};
    std::unordered_set<Hex> destinations;
    collectDestinationsAtDepthThree(board, self, from, 0, visited, destinations);
    return std::vector<Hex>(destinations.begin(), destinations.end());
}
