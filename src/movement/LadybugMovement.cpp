#include "hive/movement/LadybugMovement.h"

#include "hive/Board.h"
#include "hive/BoardGraphAdapter.h"
#include <unordered_set>
//The detail of the pattern is movement
// TODO (parte 3): implementen el movimiento de la vaquita de San Antonio
// (Ladybug), y registrarla en StrategyFactory.
//
// Es el segundo cliente de BoardGraphAdapter, y la razón por la que el
// adaptador es una clase y no código suelto dentro de Board: los dos primeros
// pasos de la vaquita son saltos sobre casilleros ocupados, es decir, caminata
// sobre el mismo grafo que usa la regla de la colmena (el que determina la
// conectividad).
//
// Al terminar, miren qué archivos hubo que tocar para agregar una pieza
// entera: la clase nueva, y el switch de la fábrica. Board, Piece y Game no se
// tocan.
//
// La regla está en include/hive/movement/LadybugMovement.h y los tests en
// tests/ladybug_movement_test.cpp.

/*
ladybug 
-- 1er paso -> ocup casilla (board.occupiedNeighbors(from)) 
-- 2do paso ->  ocup casilla (board.occupiedNeighbors(first))
-- 3er paso -> empty casilla
Igual no puedo usar ".distancesFrom" pq aplica con BFS, que es la dist minima, no todos los lugares
*/


std::vector<Hex> LadybugMovement::moves(const Board &board, const Hex &from,
                                        const Piece &self) const
{
    // TODO: devolver los destinos legales de la vaquita desde `from`.
    //(void)board;
    //(void)from;
    (void)self; //no terminamos usando self (esta para respetar los parameteos de los demas)
    //return {};
    BoardGraphAdapter grafo(board, &from);
    //Super importante! &from y no from para que no pueda volver sobre sus pasos
    //con &, node from no existe ->  graph.neighbors(A) no lo puede devolver
    //pero es masomenos lo mismo a if(second == from) continue; Si "&from" no funciona, por favor tomelo cómo el if
    
    //std::set<Hex> X; //NO COMPILA -_-
    std::unordered_set<Hex> X; //trabaja con Hash y e Hex.h dan el Hash + usamos unordered_map para BoardGraphAdapter
    //X marca el camino

    for(const Hex &first : board.occupiedNeighbors(from)){
        //cómo tecnicamente from no existe en grafo, hay que empezar en board
        for(const Hex &second : grafo.neighbors(first)){
            for(const Hex &dest : board.emptyNeighbors(second)){
                X.insert(dest);
            }
        }
    }
    //moves() son no repe -> set (resuelve asi en SpiderMovement)
    return std::vector<Hex>(X.begin(), X.end());
}