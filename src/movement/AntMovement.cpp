#include "hive/movement/AntMovement.h"

#include "hive/Board.h"

#include <unordered_set>

namespace {

// DFS con un conjunto `visited` GLOBAL: nunca se desmarca nada, al revés de la
// araña, que vuelve atrás por cada camino. Una vez que se sabe que a un casillero
// se llega, pasar de nuevo por otra ruta no descubre nada nuevo, así que cada
// casillero se explora una sola vez. Cada casillero nuevo se guarda en el momento:
// acá no hay una profundidad fija que esperar, como los 3 pasos de la araña.
void collectReachableHexes(const Board &board, const Piece &self, const Hex &cur,
                            std::unordered_set<Hex> &visited,
                            std::vector<Hex> &destinations)
{
    for (const Hex &next : neighbors(cur))
    {
        if (visited.count(next) > 0)
        {
            continue;
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
        destinations.push_back(next);
        collectReachableHexes(board, self, next, visited, destinations);
    }
}

} // namespace

std::vector<Hex> AntMovement::moves(const Board &board, const Hex &from,
                                     const Piece &self) const
{
    std::unordered_set<Hex> visited{from};
    std::vector<Hex> destinations;
    collectReachableHexes(board, self, from, visited, destinations);
    return destinations;
}
