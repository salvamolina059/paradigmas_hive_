#pragma once

#include <array>
#include <cstddef>
#include <vector>

// Coordenada hexagonal axial (q, r). Es un tipo de valor puro: no depende
// de nada del juego, y dos Hex con los mismos (q, r) son el mismo casillero.

/**
 * @brief Un casillero del tablero, en coordenadas axiales (q, r).
 *
 * No guarda nada del juego: ni pieza, ni color, ni altura. Es la dirección
 * postal de un casillero, no su contenido; quien sabe qué hay en cada uno es
 * Board.
 */
struct Hex {
    int q;
    int r;
};

/**
 * @brief Dos casilleros son iguales si tienen las mismas coordenadas.
 * @return `true` si `a` y `b` son el mismo casillero del tablero.
 */
bool operator==(const Hex& a, const Hex& b);

/**
 * @brief Los 6 desplazamientos que llevan de un casillero a cada vecino.
 *
 * Vienen en un orden de rotación fijo: cada uno está a 60 grados del
 * siguiente. `neighbors()` se construye con esto.
 *
 * Está expuesto para que todo lo que necesite razonar sobre direcciones
 * vecinas use la misma fuente —por ejemplo `Board::canSlide()`, que busca los
 * dos casilleros que flanquean un lado— en vez de deducir las direcciones con
 * rotaciones de coordenadas, que no valen en una base (q, r) no ortogonal.
 *
 * @return Referencia a las 6 direcciones, siempre en el mismo orden.
 */
const std::array<Hex, 6>& hexDirections();

/**
 * @brief Los 6 casilleros que comparten un lado con `h`.
 *
 * Salen siempre en el orden de `hexDirections()`, estén ocupados o vacíos:
 * esta función es geometría, no sabe nada del tablero.
 *
 * @param h Casillero del centro.
 * @return Los 6 vecinos de `h`.
 */
std::vector<Hex> neighbors(const Hex& h);

/**
 * @brief Cuántos pasos de casillero a casillero hay entre `a` y `b`, por el
 *        camino más corto.
 *
 * Es distancia geométrica, sin mirar qué casilleros están ocupados: no es lo
 * mismo que "en cuántos pasos puede llegar una pieza".
 *
 * @return 0 si `a` y `b` son el mismo casillero; 1 si son vecinos, etc.
 */
int distance(const Hex& a, const Hex& b);

namespace std {
template <>
struct hash<Hex> {
    /**
     * @brief Hash de un casillero, para poder usar `Hex` como clave de un
     *        `unordered_map` o `unordered_set`.
     *
     * Implementado en Hex.cpp. Es lo que hace posible que `Board` guarde sus
     * casilleros en un `unordered_map<Hex, ...>`.
     */
    std::size_t operator()(const Hex& h) const;
};
}  // namespace std
