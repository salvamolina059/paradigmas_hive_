#include "hive/MosquitoAbility.h"

#include <algorithm>

std::vector<PieceType> imitatableTypes(const Board &board, const Hex &at)
{
    std::vector<PieceType> types;
    for (const Hex &neighbor : board.occupiedNeighbors(at))
    {
        const PieceType type = board.topAt(neighbor)->type;
        if (type == PieceType::Mosquito)
        {
            continue;
        }
        if (std::find(types.begin(), types.end(), type) == types.end())
        {
            types.push_back(type);
        }
    }
    return types;
}
