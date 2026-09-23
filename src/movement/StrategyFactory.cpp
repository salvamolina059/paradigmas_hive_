#include "hive/movement/StrategyFactory.h"

#include <memory>

#include "hive/movement/QueenMovement.h"
#include "hive/movement/SpiderMovement.h"
#include "hive/movement/BeetleMovement.h"
#include "hive/movement/GrasshopperMovement.h"
#include "hive/movement/AntMovement.h"

std::unique_ptr<MovementStrategy> createMovementStrategy(PieceType type)
{
    switch (type) {
    case PieceType::Queen:
        return std::make_unique<QueenMovement>();
    case PieceType::Spider:
        return std::make_unique<SpiderMovement>();
    case PieceType::Beetle:
        return std::make_unique<BeetleMovement>();
    case PieceType::Grasshopper:
        return std::make_unique<GrasshopperMovement>();
    case PieceType::Ant:
        return std::make_unique<AntMovement>();
    case PieceType::Pillbug:
        // el bicho bolita se mueve igual que la reina
        return std::make_unique<QueenMovement>();
    case PieceType::Mosquito:
        // el mosquito no tiene movimiento propio, copia el de sus vecinos
        return nullptr;
    case PieceType::Ladybug:
        // todavia no existe, va en la parte 3
        return nullptr;
    }

    return nullptr;
}