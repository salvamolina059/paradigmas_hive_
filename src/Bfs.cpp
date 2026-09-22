#include "hive/Bfs.h"

#include <queue>

// BFS de manual: el algoritmo genérico ya está resuelto a propósito. Lo que hay
// que escribir es el adaptador que le habla (BoardGraphAdapter), no el recorrido.

std::vector<int> bfs(const std::vector<std::vector<int>> &adj, int source)
{
    const int n = static_cast<int>(adj.size());
    std::vector<bool> used(n, false);
    std::vector<int> d(n, -1);

    std::queue<int> q;
    q.push(source);
    used[source] = true;
    d[source] = 0;

    while (!q.empty())
    {
        int v = q.front();
        q.pop();
        for (int u : adj[v])
        {
            if (!used[u])
            {
                used[u] = true;
                q.push(u);
                d[u] = d[v] + 1;
            }
        }
    }

    return d;
}
