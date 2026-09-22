#include "hive/Game.h"

#include <algorithm>

#include "hive/MosquitoAbility.h"
#include "hive/movement/StrategyFactory.h"

Color Game::turn() const
{
    return turn_;
}

int Game::turnNumber() const
{
    return turnNumber_;
}

GameStatus Game::status() const
{
    return status_;
}

std::optional<Color> Game::winner() const
{
    if (status_ == GameStatus::WhiteWins)
    {
        return Color::White;
    }
    if (status_ == GameStatus::BlackWins)
    {
        return Color::Black;
    }
    return std::nullopt;
}

const Board &Game::board() const
{
    return board_;
}

const Player &Game::player(Color color) const
{
    return color == Color::White ? white_ : black_;
}

Piece *Game::lastMoved() const
{
    return lastMoved_;
}

Player &Game::currentPlayer()
{
    return turn_ == Color::White ? white_ : black_;
}

const Player &Game::currentPlayer() const
{
    return turn_ == Color::White ? white_ : black_;
}

std::vector<Move> Game::legalMoves() const
{
    std::vector<Move> moves;

    if (status_ != GameStatus::Ongoing)
    {
        return moves;
    }

    const Player &current = currentPlayer();
    const std::vector<Hex> spots = board_.legalPlacementSpots(turn_);

    const int playerTurnCount = (turnNumber_ + 1) / 2;
    const bool mustPlaceQueen = playerTurnCount == 4 && !current.hasPlacedQueen();

    const std::vector<PieceType> types =
        mustPlaceQueen ? std::vector<PieceType>{PieceType::Queen} : current.availableTypes();

    for (PieceType type : types)
    {
        for (const Hex &spot : spots)
        {
            moves.push_back(Placement{type, spot});
        }
    }

    if (current.hasPlacedQueen())
    {
        for (const Hex &hex : board_.occupiedHexes())
        {
            Piece *piece = board_.topAt(hex);
            if (piece->color != turn_)
            {
                continue;
            }
            if (piece == lastMoved_)
            {
                continue; // neither moving nor using its ability
            }

            addMoves(moves, hex, *piece);
        }
    }

    return moves;
}

void Game::addMoves(std::vector<Move> &moves, const Hex &hex, const Piece &piece) const
{
    if (piece.type == PieceType::Mosquito)
    {
        addMosquitoMoves(moves, hex, piece);
        return;
    }

    // Estar clavada impide irse del casillero, no usar una habilidad que la deja
    // donde está: por eso se chequea aparte de la rama del PillbugThrow de abajo.
    if (board_.canMove(hex))
    {
        for (const Hex &destination : piece.legalMoves(board_, hex))
        {
            moves.push_back(Movement{hex, destination});
        }
    }

    if (piece.type == PieceType::Pillbug)
    {
        for (const PillbugThrow &t : pillbugThrows(board_, hex, lastMoved_))
        {
            moves.push_back(t);
        }
    }
}

void Game::addMosquitoMoves(std::vector<Move> &moves, const Hex &hex, const Piece &piece) const
{
    if (board_.stackHeight(hex) > 1)
    {
        // En altura: piece.movement ya tiene una BeetleMovement, puesta cuando se
        // subió (ver la rama de MosquitoMovement en applyMove). Mientras está
        // arriba no elige a quién imitar: va por el mismo camino genérico que
        // cualquier otra pieza.
        if (board_.canMove(hex))
        {
            for (const Hex &destination : piece.legalMoves(board_, hex))
            {
                moves.push_back(Movement{hex, destination});
            }
        }
        return;
    }

    const std::vector<PieceType> imitating = imitatableTypes(board_, hex);

    // El bicho bolita ya no se saltea en este loop: su movimiento propio es
    // QueenMovement (ver StrategyFactory), así que imitarlo acá produce entradas
    // MosquitoMovement{imitating=Pillbug} igual que cualquier otro tipo vecino
    // —exactamente lo que ofrecería el Movement de un bicho bolita de verdad—. El
    // lanzamiento (abajo) es una habilidad aparte y adicional, no una alternativa
    // a esto.
    if (board_.canMove(hex))
    {
        for (PieceType type : imitating)
        {
            auto strategy = createMovementStrategy(type);
            for (const Hex &destination : strategy->moves(board_, hex, piece))
            {
                moves.push_back(MosquitoMovement{hex, type, destination});
            }
        }
    }

    // Tocar un bicho bolita, sin importar qué más esté tocando, alcanza para
    // prestarse el lanzamiento: igual que Game::addMoves() le ofrece a un bicho
    // bolita de verdad su Movement y su PillbugThrow a la vez.
    const bool touchesPillbug =
        std::find(imitating.begin(), imitating.end(), PieceType::Pillbug) != imitating.end();
    if (touchesPillbug)
    {
        for (const PillbugThrow &t : pillbugThrows(board_, hex, lastMoved_))
        {
            moves.push_back(t);
        }
    }
}

void Game::syncMosquitoStrategy(Piece *piece, const Hex &at) const
{
    if (piece->type != PieceType::Mosquito)
    {
        return;
    }
    piece->movement = board_.stackHeight(at) > 1 ? createMovementStrategy(PieceType::Beetle)
                                                  : nullptr;
}

bool Game::applyMove(const Move &move)
{
    if (status_ != GameStatus::Ongoing)
    {
        return false;
    }

    const std::vector<Move> legal = legalMoves();
    if (std::find(legal.begin(), legal.end(), move) == legal.end())
    {
        return false;
    }

    if (std::holds_alternative<Placement>(move))
    {
        const auto &placement = std::get<Placement>(move);

        pieces_.push_back(std::make_unique<Piece>(turn_, placement.type));
        Piece *piece = pieces_.back().get();

        board_.place(piece, placement.destination);
        currentPlayer().place(placement.type);
        lastMoved_ = piece; // the placed piece replaces whatever moved before it
    }
    else if (std::holds_alternative<Movement>(move))
    {
        const auto &movement = std::get<Movement>(move);
        Piece *piece = board_.topAt(movement.from);

        board_.move(movement.from, movement.destination);
        lastMoved_ = piece;
        syncMosquitoStrategy(piece, movement.destination);
    }
    else if (std::holds_alternative<PillbugThrow>(move))
    {
        const auto &t = std::get<PillbugThrow>(move);
        Piece *victim = board_.topAt(t.victim);

        board_.move(t.victim, t.destination);
        lastMoved_ = victim; // the victim moved, not the thrower
    }
    else if (std::holds_alternative<MosquitoMovement>(move))
    {
        const auto &m = std::get<MosquitoMovement>(move);
        Piece *piece = board_.topAt(m.from);

        board_.move(m.from, m.destination);
        lastMoved_ = piece; // the mosquito itself moved
        syncMosquitoStrategy(piece, m.destination);
    }

    turn_ = (turn_ == Color::White) ? Color::Black : Color::White;
    ++turnNumber_;
    updateStatus();

    return true;
}

void Game::updateStatus()
{
    const bool whiteSurrounded = isQueenSurrounded(Color::White);
    const bool blackSurrounded = isQueenSurrounded(Color::Black);

    if (whiteSurrounded && blackSurrounded)
    {
        status_ = GameStatus::Draw;
    }
    else if (whiteSurrounded)
    {
        status_ = GameStatus::BlackWins;
    }
    else if (blackSurrounded)
    {
        status_ = GameStatus::WhiteWins;
    }
}

bool Game::isQueenSurrounded(Color color) const
{
    for (const Hex &hex : board_.occupiedHexes())
    {
        if (board_.contains(hex, color, PieceType::Queen))
        {
            return board_.isSurrounded(hex);
        }
    }
    return false; // that color hasn't placed its queen yet
}
