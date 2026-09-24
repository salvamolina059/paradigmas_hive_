//Mini nota, van a ver varios comentarios que son con "->"
//son para guiarme para programar y si hay algun error en el razonamiento se pueda detectar
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
// del tablero están usando el adaptador... (->Vino bien, gracias)
//
// Los tests están en tests/board_graph_adapter_test.cpp, y los de
// Board::canMove()/isConnected() —que usan esta clase— en
// tests/board_connectivity_test.cpp.
// ============================================================================

    
BoardGraphAdapter::BoardGraphAdapter(const Board &board, const Hex *ignoring)
{
    // TODO: construir el grafo de casilleros ocupados, salteando `ignoring`
    // si no es nullptr.
    //->este es el constructor, OJO no f virtuales. (todavia no derivadas)
    //no explicit, aunque ninguna de las 2 se necesita para aca
    auto occupied = board.occupiedHexes();
    //sin auto tendriamos que hacer algo mas tipo for([contenedor] <...> :: iter it cont. (...))
    for (const Hex &h : occupied){ //auto resuelve :)
        if(ignoring != nullptr && h == *ignoring)
            continue;
        //si no es nullptr, asign id
        int id = static_cast<int>(nodeToHex_.size()); //primer Hex es 0, segundo 1, etc
        hexToNode_[h] = id;
        /*
        De esta forma, si hay que ignorar a un nodo, no rompe la enumeracion de los ID
        y nos queda un hex con todos nodos no aislados
        */
        nodeToHex_.push_back(h);
    }
    adjacency_.resize(nodeToHex_.size());
    //adjacency_ = {{},{},...,{}}
    for (const Hex &h : nodeToHex_){
        int from = hexToNode_.at(h);
        //dps lo usamos en la Vaquita
        for (const Hex &neighbor : board.occupiedNeighbors(h)){
            auto it = hexToNode_.find(neighbor);
            //board.isOccupied(neighbor) podría ser ignoring.y justamente lo queremos ignorar...
            if(it!=hexToNode_.end()){
                //+Edge
                adjacency_[from].push_back(it->second); //it->second = Node
            }
        }
    }
    //(void)board;
    //(void)ignoring;
}

std::vector<Hex> BoardGraphAdapter::neighbors(const Hex &h) const
{
    // TODO: devolver los vecinos de `h` dentro de este grafo.
    //es para hacer "graph.neighbors(Hex{0,0})"
    //Hex->ID (nodo)->ID vec->vec (Hex). Ah yes, it's all coming together.
    auto it = hexToNode_.find(h);
    if(it == hexToNode_.end()) return {};
    int node = it->second;
    std::vector<Hex> result;
    for(int neighborID : adjacency_[node]){
        result.push_back(nodeToHex_[neighborID]);
    }
    //(void)h;
    //return {};
    return result;
}

std::unordered_map<Hex, int> BoardGraphAdapter::distancesFrom(const Hex &source) const
{
    // TODO: distancias en saltos desde `source` a cada nodo, usando bfs().
    //Nota: bfs(adjacency_, source) return -1 if inalcanzable
    //Hex->nodo->BFS->Hex

    //source Hex -> source Int
    auto it = hexToNode_.find(source);
    if(it == hexToNode_.end()) return {}; //
    std::vector<int> distances = bfs(adjacency_, it->second); //que raro que esperen ";" las funciones
    //BFS habla en IDs! pero OJO tenemos que return unordered_map<Hex, int>
    std::unordered_map<Hex, int> result;
    for(std::size_t i = 0; i < distances.size(); i++){
        result[nodeToHex_[i]] = distances[i]; //los inalcanzables siguen apareciendo cómo -1
    }
    //(void)source;
    //return {};
    return result;
}

bool BoardGraphAdapter::isConnected() const
{
    // TODO: devolver si todos los nodos del grafo se alcanzan entre sí.
    //mientras que ninguno sea -1 todos conectados (por esto tmb distancesFrom guarda los -1)
    if(adjacency_.empty()) return true; //por consigna, el vacio es conectado 
    std::vector<int> dists = bfs(adjacency_, 0); //si esta conectado, no matter start
    //ya existen distance y distances.
    for (int dist : dists){
        if(dist == -1) return false;
    }
    return true;
}
//con todo esto Board -