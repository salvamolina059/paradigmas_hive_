#pragma once

/** @brief Los dos jugadores: las blancas arrancan. */
enum class Color {
    White,
    Black,
};

/**
 * @brief Los 8 tipos de pieza del juego, con las tres expansiones incluidas
 *        (mosquito, vaquita de San Antonio y bicho bolita).
 *
 * Cada tipo se mueve distinto, y esa diferencia no vive acá: vive en una
 * clase por tipo (ver MovementStrategy.h) que `createMovementStrategy()`
 * sabe elegir.
 */
enum class PieceType {
    Queen,
    Spider,
    Beetle,
    Grasshopper,
    Ant,
    Mosquito,
    Ladybug,
    Pillbug,
};
