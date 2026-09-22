#include "hive/BoardGraphAdapter.h"

#include <cstddef>

#include "hive/Bfs.h"

// ============================================================================
// TODO (parte 3): implementen el adaptador entre el tablero y bfs().
//
// bfs() (include/hive/Bfs.h) es una biblioteca genérica: no sabe nada de Hive
// ni de hexágonos. Habla de nodos numerados 0..n-1 y de listas de adyacencia.
// Board, del otro lado, habla de Hex y de casilleros ocupados. Ninguno de los
// dos se puede cambiar: esta clase es la que traduce entre los dos idiomas.
//
// El header declara solo la parte pública. Tienen que decidir el ESTADO: para
// traducir en las dos direcciones hace falta guardar algo en el constructor.
// Ese estado va en la sección `private:` del header.
//
// Para pensar: `ignoring` es un parámetro del constructor, no un método que
// saque la pieza del tablero y después la vuelva a poner. ¿Por qué conviene
// que sea así? ¿Qué podría salir mal con la otra versión? Fíjense qué partes
// del tablero están usando el adaptador...
//
// Los tests están en tests/board_graph_adapter_test.cpp, y los de
// Board::canMove()/isConnected() —que usan esta clase— en
// tests/board_connectivity_test.cpp.
// ============================================================================

BoardGraphAdapter::BoardGraphAdapter(const Board &board, const Hex *ignoring)
{
    // TODO: construir el grafo de casilleros ocupados, salteando `ignoring`
    // si no es nullptr.
    (void)board;
    (void)ignoring;
}

std::vector<Hex> BoardGraphAdapter::neighbors(const Hex &h) const
{
    // TODO: devolver los vecinos de `h` dentro de este grafo.
    (void)h;
    return {};
}

std::unordered_map<Hex, int> BoardGraphAdapter::distancesFrom(const Hex &source) const
{
    // TODO: distancias en saltos desde `source` a cada nodo, usando bfs().
    (void)source;
    return {};
}

bool BoardGraphAdapter::isConnected() const
{
    // TODO: devolver si todos los nodos del grafo se alcanzan entre sí.
    return false;
}
