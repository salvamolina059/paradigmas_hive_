#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "hive/Board.h"
#include "hive/Enums.h"
#include "hive/Move.h"
#include "hive/Piece.h"
#include "hive/Player.h"

enum class GameStatus {
    Ongoing,
    WhiteWins,
    BlackWins,
    Draw,
};

// El que lleva los turnos y hace cumplir las reglas. Es dueño de todas las
// piezas que se colocaron alguna vez: se construyen recién cuando se aplica una
// colocación, no hay un pozo de piezas sin colocar esperando. Lo que decide si
// una colocación de cierto tipo es legal es Player::remaining().
//
// A un jugador que todavía no colocó su reina (Player::hasPlacedQueen()) no se
// le ofrece ningún Movement ni PillbugThrow: le queda solo Placement. Vale para
// todas sus piezas ya colocadas, no solo para la reina.
//
// lastMoved_ es la pieza que actuó el turno pasado, y no puede volver a moverse
// ni ser lanzada este turno. Todas las movidas lo reemplazan, ninguna lo deja
// como estaba: Movement (la pieza que se movió), PillbugThrow (la víctima, no el
// que lanzó, que nunca cambió de lugar), MosquitoMovement (el mosquito) y
// Placement (la pieza que se acaba de colocar). Que Placement también lo
// reemplace es lo que evita que el dato quede viejo: sin eso, la pieza que se
// movió hace varios turnos seguiría excluida a través de cualquier cantidad de
// colocaciones, en vez del único turno que la restricción quiere cubrir.
//
// El mosquito es el caso especial de legalMoves() (ver addMosquitoMoves): a
// nivel del piso ofrece un MosquitoMovement por cada tipo que pueda imitar (o
// PillbugThrow, si toca exactamente un bicho bolita), pero arriba de una pila
// ofrece un Movement común con el movimiento del escarabajo, sin elección de a
// quién imitar. Llegó ahí imitando al escarabajo para subirse, y sigue siendo un
// escarabajo hasta que baja; al bajar, applyMove (vía syncMosquitoStrategy) lo
// vuelve a nullptr, así la próxima consulta vuelve a elegir a quién imitar desde
// cero.
class Game {
public:
    Color turn() const;
    int turnNumber() const;
    GameStatus status() const;

    // Si la partida sigue en juego o terminó empatada, no hay ganador: nullopt.
    std::optional<Color> winner() const;

    const Board &board() const;
    const Player &player(Color color) const;

    // La pieza que actuó el turno pasado (nullptr solo antes de la primera movida
    // de la partida). Este turno no puede volver a moverse ni ser lanzada: es la
    // restricción que trae el bicho bolita, ver el comentario de la clase.
    Piece *lastMoved() const;

    // Todas las movidas legales del jugador de turno. Vacío si status() ya no es
    // Ongoing: la partida terminó y no hay nada más que ofrecer.
    //
    // Si sigue en juego: las colocaciones (los casilleros vacíos que da
    // Board::legalPlacementSpots, restringidas a PieceType::Queen solamente si este es
    // el 4º turno de ese jugador —(turnNumber_ + 1) / 2— y todavía no la colocó) y, una vez que la
    // reina está en el tablero, los Movement y PillbugThrow de cada pieza ya
    // colocada que esté habilitada: del jugador de turno, no clavada, y que no sea
    // lastMoved_.
    std::vector<Move> legalMoves() const;

    // Chequea que `move` sea una de las movidas que devuelve legalMoves() y la
    // aplica: toca board_ y la mano del jugador de turno, avanza turn_ y
    // turnNumber_, y recalcula status_ (ver updateStatus).
    //
    // Devuelve false, sin modificar nada, si la movida no es legal o si la partida
    // ya terminó.
    bool applyMove(const Move &move);

private:
    Player &currentPlayer();
    const Player &currentPlayer() const;

    // Agrega a `moves` las movidas legales de `piece`, que está en `hex`. El
    // mosquito va por un camino aparte: ver el comentario de la clase.
    void addMoves(std::vector<Move> &moves, const Hex &hex, const Piece &piece) const;
    void addMosquitoMoves(std::vector<Move> &moves, const Hex &hex, const Piece &piece) const;

    // Para cualquier pieza que no sea el mosquito, no hace nada. Para un mosquito
    // que acaba de caer en `at`: si quedó arriba de una pila (stackHeight(at) > 1),
    // le deja fija una BeetleMovement; si quedó a nivel del piso, lo vuelve a
    // nullptr.
    //
    // Cubre las dos puntas: cuando se sube (con un MosquitoMovement) y cuando más
    // tarde baja (con un Movement común, porque un mosquito en altura usa el
    // moves() del escarabajo, que permite bajar a un casillero vacío).
    void syncMosquitoStrategy(Piece *piece, const Hex &at) const;

    // Recalcula status_ de cero contra el tablero actual: las dos reinas rodeadas es
    // empate, una sola rodeada es victoria del otro color, ninguna es que la partida
    // sigue. Una partida terminada no se destermina, así que esto nunca tiene que
    // volver un status_ ya decidido a Ongoing.
    void updateStatus();

    // Si la reina de `color` está colocada y su casillero cumple
    // Board::isSurrounded. El casillero se busca con Board::contains, porque la
    // reina puede estar enterrada abajo de un escarabajo. Si ese color todavía no
    // colocó su reina, false.
    bool isQueenSurrounded(Color color) const;

    Board board_;
    Player white_{Color::White};
    Player black_{Color::Black};
    Color turn_ = Color::White;
    int turnNumber_ = 1;
    GameStatus status_ = GameStatus::Ongoing;
    Piece *lastMoved_ = nullptr;
    std::vector<std::unique_ptr<Piece>> pieces_;
};
