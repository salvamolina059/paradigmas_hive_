#pragma once

#include <vector>

/**
 * @brief BFS genérico, independiente de Hive y de `Hex`: trabaja sobre
 *        cualquier grafo expresado como lista de adyacencia de nodos numerados
 *        en el rango `[0, adj.size())`.
 *
 * La idea es manejarlo a través de un adaptador que traduzca un grafo de un
 * dominio concreto —por ejemplo los casilleros ocupados de `Board`— a esta
 * forma.
 *
 * @param adj    Lista de adyacencia: `adj[i]` son los vecinos del nodo `i`.
 * @param source Nodo desde donde se miden las distancias.
 * @return Las distancias desde `source`: `result[i]` es la cantidad de saltos
 *         del camino más corto de `source` a `i`, o -1 si `i` es inalcanzable.
 *         `result[source]` siempre vale 0.
 */
std::vector<int> bfs(const std::vector<std::vector<int>>& adj, int source);
