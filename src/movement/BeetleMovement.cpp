#include "hive/movement/BeetleMovement.h"

#include "hive/Board.h"
#include "hive/Hex.h"

std::vector<Hex> BeetleMovement::moves(const Board &board, const Hex &from,
                                        const Piece &self) const
{
    std::vector<Hex> destinos;

    for (const Hex &vecino : neighbors(from)) {
        // se mueve por arriba si ya esta sobre una pila o si se sube a una pieza
        bool porArriba = board.stackHeight(from) > 1 || board.isOccupied(vecino);

        if (porArriba) {
            // por arriba no hay hueco que lo trabe y siempre toca la colmena
            destinos.push_back(vecino);
        } else if (board.canSlide(from, vecino) && board.wouldStayAttached(vecino, self)) {
            // piso a piso: mismas reglas que la reina
            destinos.push_back(vecino);
        }
    }

    return destinos;
}