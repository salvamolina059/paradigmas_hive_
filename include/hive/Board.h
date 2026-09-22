#pragma once

#include <cstddef>
#include <unordered_map>
#include <vector>

#include "hive/Enums.h"
#include "hive/Hex.h"
#include "hive/Piece.h"

/**
 * @brief El tablero: qué pieza hay en cada casillero, y las dos reglas
 *        generales que valen para todas.
 *
 * Guarda punteros a piezas que no le pertenecen: el dueño de las piezas es
 * `Game`. Por eso los tests de tablero pueden crear `Piece` en el stack, que
 * `Board` no va a destruir.
 *
 * `place()` y `move()` apilan y desapilan sin preguntar: `Board` no se fija si
 * alguien tiene derecho a caer ahí. Eso lo decide quien llama, consultando
 * antes `canSlide()`, `wouldStayAttached()` o `canMove()`.
 */

class Board
{
public:
    /**
     * @brief Apila `piece` arriba de lo que haya en `at`.
     * @param piece La pieza a colocar; el tablero no pasa a ser su dueño.
     * @param at    El casillero donde ponerla, esté vacío u ocupado.
     */
    void place(Piece *piece, const Hex &at);

    /**
     * @brief Saca la pieza de arriba de `from` y la apila en `to`.
     *
     * Si `to` está ocupado, queda arriba de la pila: así es como sube un
     * escarabajo. No valida la movida (ver el comentario de la clase).
     */
    void move(const Hex &from, const Hex &to);

    /**
     * @brief Saca del tablero la pieza de arriba de `at` y la devuelve.
     * @return La pieza que estaba arriba, o `nullptr` si el casillero estaba
     *         vacío.
     */
    Piece *remove(const Hex &at);

    /**
     * @brief La pieza que está arriba de todo en `at`.
     *
     * Es la única que puede moverse desde ese casillero, y la que cuenta para
     * saber de quién es: un escarabajo blanco arriba de una reina negra hace
     * que ese casillero sea, para el resto del juego, blanco.
     *
     * @return La pieza de arriba, o `nullptr` si `at` está vacío.
     */
    Piece *topAt(const Hex &at) const;

    /** @brief Si hay al menos una pieza en `at`. */
    bool isOccupied(const Hex &at) const;

    /**
     * @brief Cuántas piezas hay apiladas en `at`.
     * @return 0 si está vacío, 1 si hay una sola pieza, más si hay una pila.
     */
    std::size_t stackHeight(const Hex &at) const;

    /**
     * @brief Todos los casilleros que tienen al menos una pieza, es decir la
     *        colmena entera.
     *
     * El orden no está garantizado y puede cambiar entre corridas: si hace
     * falta un orden, hay que ordenar.
     */
    std::vector<Hex> occupiedHexes() const;

    /**
     * @brief Los vecinos de `at` que no tienen ninguna pieza.
     *
     * El punto de partida de casi todas las estrategias que caminan por el
     * piso: la reina, la araña y la hormiga solo pueden pisar casilleros
     * vacíos.
     */
    std::vector<Hex> emptyNeighbors(const Hex &at) const;

    /** @brief Los vecinos de `at` que tienen al menos una pieza. */
    std::vector<Hex> occupiedNeighbors(const Hex &at) const;

    /**
     * @brief Si la colmena está entera: todos los casilleros ocupados se
     *        alcanzan entre sí.
     */
    bool isConnected() const;

    /**
     * @brief La mitad de la regla de la colmena que le toca al casillero de
     *        partida: si sacar la pieza de arriba de `at` no rompería la regla,
     *        o sea si `isConnected()` seguiría valiendo sacando `at`.
     *
     * Una pieza que no cumple esto está *clavada*: no se puede mover a ningún
     * lado, porque al levantarla la colmena queda partida en dos.
     *
     * Una pieza arriba de una pila nunca está clavada (`stackHeight(at) > 1`):
     * el casillero sigue ocupado por la que está abajo, así que la colmena no
     * se entera de que se fue.
     */
    bool canMove(const Hex &at) const;

    /**
     * @brief La regla del hueco: si una pieza puede deslizarse de `from` a `to`
     *        sin tener que pasar por un hueco tapado de los dos lados.
     *
     * El lado `from`-`to` tiene dos casilleros que lo flanquean, los que son
     * vecinos de los dos a la vez. Alcanza con que uno esté vacío: si los dos
     * están ocupados, la pieza no entra por ahí, aunque `to` esté libre.
     *
     * @return `false` también si `to` no es vecino de `from`: no existe tal
     *         lado, así que no hay hueco que pueda estar abierto ni tapado.
     */
    bool canSlide(const Hex &from, const Hex &to) const;

    /**
     * @brief Si `at` toca la colmena, haciendo de cuenta que `excluding` ya no
     *        está en el tablero.
     *
     * La pregunta de fondo es "¿este casillero toca alguna pieza?", pero hecha
     * *durante* una movida: la pieza que se está moviendo no se cuenta como
     * contacto consigo misma.
     *
     * Un vecino cuya única pieza es `excluding` no cuenta. Pero si ese vecino
     * es una pila (`stackHeight > 1`), sacar `excluding` no lo deja vacío, así
     * que sigue contando.
     *
     * @param excluding La pieza que se considera levantada, o `nullptr` si no
     *                  hay ninguna.
     */
    bool hasOccupiedNeighborExcluding(const Hex &at, const Piece *excluding) const;

    /**
     * @brief La mitad de la regla de la colmena que le toca al destino: si
     *        `moving`, al caer en `to`, seguiría pegada al resto de la colmena.
     *
     * Es `hasOccupiedNeighborExcluding(to, &moving)`, más la excepción: si
     * `moving` es la única pieza del tablero, no hay "resto de la colmena" al
     * que quedar pegada, y cualquier destino vale.
     *
     * No se fija si `moving` está clavada en el casillero donde está ahora: esa
     * es la otra mitad, `canMove(from)`.
     *
     * Esta excepción vive acá, y no repetida en cada estrategia, justamente
     * para que todas las piezas que se deslizan la cumplan igual.
     */
    bool wouldStayAttached(const Hex &to, const Piece &moving) const;

    /**
     * @brief Los casilleros vacíos donde `color` puede colocar una pieza nueva:
     *        pegados a la colmena y sin tocar ninguna pieza del rival.
     *
     * Las dos primeras colocaciones de la partida son la excepción, y se
     * detectan sin llevar la cuenta de los turnos —al tablero se le agregan
     * piezas y nunca se le sacan, así que la cantidad de casilleros ocupados
     * alcanza—:
     *
     * - Tablero vacío: es la primera colocación de la partida, no hay
     *   restricción de adyacencia y por convención va al origen.
     * - Exactamente un casillero ocupado: es la segunda, tiene que ir pegada a
     *   esa única pieza, y la regla de no tocar al rival se saltea. Esa pieza
     *   es necesariamente del rival, así que aplicarla devolvería cero lugares.
     */
    std::vector<Hex> legalPlacementSpots(Color color) const;

    /**
     * @brief Si los 6 vecinos de `at` están ocupados.
     *
     * Es la condición de victoria: `Game` la consulta sobre el casillero de
     * cada reina. No le importa de quién son las piezas que rodean —una reina
     * rodeada por sus propias piezas también pierde.
     */
    bool isSurrounded(const Hex &at) const;

    /**
     * @brief Si en la pila de `at` hay una pieza de `color` y `type`, a
     *        cualquier altura y no solo arriba.
     *
     * Sirve para encontrar una reina enterrada abajo de un escarabajo: para la
     * reina lo que importa es estar rodeada, no estar arriba.
     */
    bool contains(const Hex &at, Color color, PieceType type) const;

private:
    std::unordered_map<Hex, std::vector<Piece *>> cells_;
};
