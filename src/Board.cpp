#include "hive/Board.h"

#include <unordered_set>

#include "hive/BoardGraphAdapter.h"

void Board::place(Piece *piece, const Hex &at)
{
    cells_[at].push_back(piece);
}

void Board::move(const Hex &from, const Hex &to)
{
    Piece *piece = remove(from);
    place(piece, to);
}

Piece *Board::remove(const Hex &at)
{
    auto &stack = cells_[at];
    if (stack.empty())
    {
        return nullptr;
    }
    Piece *piece = stack.back();
    stack.pop_back();
    return piece;
}

Piece *Board::topAt(const Hex &at) const
{
    auto it = cells_.find(at);
    if (it == cells_.end() || it->second.empty())
    {
        return nullptr;
    }
    return it->second.back();
}

bool Board::isOccupied(const Hex &at) const
{
    auto it = cells_.find(at);
    return it != cells_.end() && !it->second.empty();
}

std::size_t Board::stackHeight(const Hex &at) const
{
    auto it = cells_.find(at);
    return it != cells_.end() ? it->second.size() : 0;
}

std::vector<Hex> Board::occupiedHexes() const
{
    std::vector<Hex> hexes;
    for (const auto &pair : cells_)
    {
        if (!pair.second.empty())
        {
            hexes.push_back(pair.first);
        }
    }
    return hexes;
}

std::vector<Hex> Board::emptyNeighbors(const Hex &at) const
{
    std::vector<Hex> empty;
    for (const Hex &neighbor : neighbors(at))
    {
        if (!isOccupied(neighbor))
        {
            empty.push_back(neighbor);
        }
    }
    return empty;
}

std::vector<Hex> Board::occupiedNeighbors(const Hex &at) const
{
    std::vector<Hex> occupied;
    for (const Hex &neighbor : neighbors(at))
    {
        if (isOccupied(neighbor))
        {
            occupied.push_back(neighbor);
        }
    }
    return occupied;
}

bool Board::isConnected() const
{
    auto adapter = BoardGraphAdapter(*this, nullptr);
    return adapter.isConnected();
}

bool Board::canMove(const Hex &at) const
{
    if (stackHeight(at) > 1)
    {
        return true;
    }
    auto adapter = BoardGraphAdapter(*this, &at);
    return adapter.isConnected();
}

bool Board::canSlide(const Hex &from, const Hex &to) const
{
    const int dq = to.q - from.q;
    const int dr = to.r - from.r;

    // Buscamos cuál de las 6 direcciones canónicas es from->to, así los dos
    // casilleros que flanquean el lado salen del mismo orden fijo de direcciones
    // que usa neighbors(), en vez de deducirlos con una fórmula de rotación de
    // coordenadas: esas fórmulas no valen en la base axial (q, r), que no es
    // ortogonal.
    const auto &dirs = hexDirections();
    int index = -1;
    for (std::size_t i = 0; i < dirs.size(); ++i)
    {
        if (dirs[i].q == dq && dirs[i].r == dr)
        {
            index = static_cast<int>(i);
            break;
        }
    }
    if (index == -1)
    {
        return false; // `to` is not a neighbor of `from`
    }

    const Hex &d1 = dirs[(index + 1) % 6];
    const Hex &d2 = dirs[(index + 5) % 6]; // index - 1, wrapped
    const Hex flank1{from.q + d1.q, from.r + d1.r};
    const Hex flank2{from.q + d2.q, from.r + d2.r};

    return !isOccupied(flank1) || !isOccupied(flank2);
}

bool Board::hasOccupiedNeighborExcluding(const Hex &at, const Piece *excluding) const
{
    for (const Hex &neighbor : neighbors(at))
    {
        Piece *top = topAt(neighbor);
        if (top == nullptr)
        {
            continue;
        }
        if (top != excluding || stackHeight(neighbor) > 1)
        {
            return true;
        }
    }
    return false;
}

bool Board::wouldStayAttached(const Hex &to, const Piece &moving) const
{
    if (occupiedHexes().size() <= 1)
    {
        return true;
    }
    return hasOccupiedNeighborExcluding(to, &moving);
}

bool Board::isSurrounded(const Hex &at) const
{
    return occupiedNeighbors(at).size() == 6;
}

bool Board::contains(const Hex &at, Color color, PieceType type) const
{
    auto it = cells_.find(at);
    if (it == cells_.end())
    {
        return false;
    }
    for (Piece *piece : it->second)
    {
        if (piece->color == color && piece->type == type)
        {
            return true;
        }
    }
    return false;
}

std::vector<Hex> Board::legalPlacementSpots(Color color) const
{
    const std::vector<Hex> occupied = occupiedHexes();

    if (occupied.empty())
    {
        return {Hex{0, 0}};
    }

    if (occupied.size() == 1)
    {
        return emptyNeighbors(occupied.front());
    }

    std::unordered_set<Hex> candidates;
    for (const Hex &hex : occupied)
    {
        for (const Hex &empty : emptyNeighbors(hex))
        {
            candidates.insert(empty);
        }
    }

    std::vector<Hex> result;
    for (const Hex &candidate : candidates)
    {
        bool touchesEnemy = false;
        for (const Hex &neighbor : occupiedNeighbors(candidate))
        {
            if (topAt(neighbor)->color != color)
            {
                touchesEnemy = true;
                break;
            }
        }
        if (!touchesEnemy)
        {
            result.push_back(candidate);
        }
    }
    return result;
}
