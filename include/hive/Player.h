#pragma once

#include <unordered_map>
#include <vector>

#include "hive/Enums.h"

/**
 * @brief El color de un jugador y su mano: las piezas que todavía no colocó,
 *        contadas por tipo.
 *
 * Arranca con la mano completa, juego base más expansiones: 1 reina, 2 arañas,
 * 2 escarabajos, 3 saltamontes, 3 hormigas, 1 mosquito, 1 vaquita de San
 * Antonio y 1 bicho bolita.
 */
class Player {
public:
    /**
     * @brief Crea un jugador de `color` con la mano completa, sin nada
     *        colocado.
     */
    explicit Player(Color color);

    /** @brief El color con el que se construyó el jugador. */
    Color color() const;

    /**
     * @brief Cuántas piezas de `type` le quedan al jugador sin colocar.
     * @return 0 cuando ya colocó todas las de ese tipo.
     */
    int remaining(PieceType type) const;

    /**
     * @brief Los tipos que todavía se pueden colocar, es decir los que tienen
     *        `remaining() > 0`.
     */
    std::vector<PieceType> availableTypes() const;

    /**
     * @brief Registra que el jugador colocó una pieza de `type`: le descuenta 1
     *        a la mano.
     *
     * No valida nada (ver el comentario de la clase): si lo llaman con un tipo
     * que ya se agotó, el que se equivocó es quien llamó.
     */
    void place(PieceType type);

    /** @brief Si el jugador ya colocó su reina. */
    bool hasPlacedQueen() const;

private:
    // El estado va acá: qué datos necesita guardar un jugador para poder
    // responder a los métodos de arriba, y por qué conviene que sean privados.
};
