#include "hive/Player.h"

Player::Player(Color color)
    : color_(color),
      mano_{
          {PieceType::Queen, 1},
          {PieceType::Spider, 2},
          {PieceType::Beetle, 2},
          {PieceType::Grasshopper, 3},
          {PieceType::Ant, 3},
          {PieceType::Mosquito, 1},
          {PieceType::Ladybug, 1},
          {PieceType::Pillbug, 1},
      }
{
}

Color Player::color() const
{
    return color_;
}

int Player::remaining(PieceType tipo) const
{
    return mano_.at(tipo);
}

std::vector<PieceType> Player::availableTypes() const
{
    // lista fija para que el menu salga siempre en el mismo orden
    // (el unordered_map no garantiza ningun orden)
    const PieceType todos[] = {
        PieceType::Queen,
        PieceType::Spider,
        PieceType::Beetle,
        PieceType::Grasshopper,
        PieceType::Ant,
        PieceType::Mosquito,
        PieceType::Ladybug,
        PieceType::Pillbug,
    };

    std::vector<PieceType> disponibles;
    for (PieceType tipo : todos) {
        if (remaining(tipo) > 0) {
            disponibles.push_back(tipo);
        }
    }
    return disponibles;
}

void Player::place(PieceType tipo)
{
    mano_[tipo] -= 1;
}

bool Player::hasPlacedQueen() const
{
    // no guardo un bool aparte: si no quedan reinas en la mano, ya la puso
    return remaining(PieceType::Queen) == 0;
}