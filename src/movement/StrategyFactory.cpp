#include "hive/movement/StrategyFactory.h"

// TODO (parte 2): implementen la fábrica de estrategias.
//
// Es el único lugar del programa que sabe qué clase corresponde a cada
// PieceType: Piece la usa en su constructor y nunca menciona una estrategia
// concreta. Las clases concretas están en include/hive/movement/ (hay que
// incluir las que se usen).
//
// Dos casos no son un tipo nuevo y están explicados en StrategyFactory.h:
// Pillbug y Mosquito. Lean ese comentario antes de escribir el switch.
//
// Los tests están en tests/strategy_factory_test.cpp. El caso de Ladybug es
// de la parte 3: hasta entonces esa clase no existe.

std::unique_ptr<MovementStrategy> createMovementStrategy(PieceType type)
{
    // TODO: devolver la estrategia que corresponde a `type`.
    (void)type;
    return nullptr;
}
