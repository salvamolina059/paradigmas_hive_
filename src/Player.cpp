#include "hive/Player.h"

// ============================================================================
// TODO (parte 1a): implementen cada método según el contrato documentado en
// include/hive/Player.h.
//
// El header declara solo la parte pública: lo que el resto del programa (y los
// tests) puede usar. Tienen que decidir el ESTADO: qué datos necesita guardar
// un jugador para poder responder a estos métodos, y por qué conviene que sean
// privados. Ese estado va en la sección `private:` del header.
// ============================================================================

Player::Player(Color color)
{
    // TODO: dejar al jugador en su estado inicial: su color, y la mano
    // completa de piezas sin colocar. Las cantidades de cada tipo están
    // documentadas en Player.h.
    (void)color;
}

Color Player::color() const
{
    // TODO: devolver el color del jugador.
    return Color::White;
}

int Player::remaining(PieceType type) const
{
    // TODO: devolver cuántas piezas de ese tipo quedan sin colocar.
    (void)type;
    return -1;
}

std::vector<PieceType> Player::availableTypes() const
{
    // TODO: devolver los tipos que todavía se pueden colocar.
    return {};
}

void Player::place(PieceType type)
{
    // TODO: registrar que se colocó una pieza de ese tipo.
    (void)type;
}

bool Player::hasPlacedQueen() const
{
    // TODO: devolver si la reina ya fue colocada.
    //
    // Para pensar: ¿conviene guardar esto en un dato aparte, o se puede
    // deducir del estado que ya existe? ¿Qué pasa si alguien agrega otra
    // forma de sacar piezas de la mano y se olvida de actualizar el dato?
    return false;
}
