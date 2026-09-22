#pragma once

#include <unordered_map>
#include <vector>

#include "hive/Board.h"
#include "hive/Hex.h"

/**
 * @brief Traduce entre el idioma de `Board` —casilleros y ocupación— y el de
 *        `bfs()` —nodos numerados y listas de adyacencia—, y traduce las
 *        respuestas de vuelta a casilleros.
 *
 * Quien lo usa nunca ve un número de nodo: esa es la única razón por la que
 * esta clase existe.
 *
 * Modela el grafo de los casilleros *ocupados*, con una arista entre dos
 * ocupados que sean vecinos. Con eso se contestan la regla de la colmena y los
 * dos pasos por arriba de la vaquita de San Antonio (`distancesFrom()`). Otro
 * grafo distinto —por ejemplo el del perímetro de casilleros vacíos que camina
 * la hormiga— necesitaría su propio adaptador, no un agregado a este.
 *
 * Se arma de una sola vez al construirse, a partir de una foto del tablero: no
 * queda ningún vínculo vivo con el `Board`, así que si el tablero cambia
 * después, el adaptador queda viejo. Conviene construir uno nuevo por consulta.
 */
class BoardGraphAdapter {
public:
    /**
     * @brief Arma el grafo de los casilleros ocupados de `board`.
     *
     * @param board    El tablero del que se saca la foto.
     * @param ignoring Si no es `nullptr`, este casillero y todas sus aristas
     *                 quedan afuera del grafo, como si su pieza se hubiera
     *                 levantado del tablero.
     *
     * @note `ignoring` no tiene nada de Hive: es el subgrafo inducido sin un
     *       nodo, a secas. `Board` lo usa para preguntar si levantar una pieza
     *       partiría la colmena, sin tener que sacarla y volverla a poner.
     */
    explicit BoardGraphAdapter(const Board& board, const Hex* ignoring = nullptr);

    /**
     * @brief Los vecinos ocupados de `h` dentro de este grafo, es decir sin
     *        contar el casillero ignorado.
     * @return Lista vacía si `h` no forma parte del grafo.
     */
    std::vector<Hex> neighbors(const Hex& h) const;

    /**
     * @brief Distancia en saltos desde `source` a cada nodo del grafo.
     *
     * @return Un mapa de casillero a distancia. Los casilleros que no se
     *         alcanzan aparecen con distancia -1, igual que en `bfs()`.
     */
    std::unordered_map<Hex, int> distancesFrom(const Hex& source) const;

    /**
     * @brief Si todos los nodos del grafo se alcanzan entre sí.
     * @return `true` para el grafo vacío: no hay nada que pueda estar separado.
     */
    bool isConnected() const;

private:
    // El estado va acá: qué hace falta guardar en el constructor para poder
    // traducir en las dos direcciones (Hex -> nodo, nodo -> Hex) y para
    // responder las consultas de arriba sin volver a mirar el Board.
};
