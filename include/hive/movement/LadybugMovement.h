#pragma once

#include "hive/movement/MovementStrategy.h"

/**
 * @brief Vaquita de San Antonio: exactamente 3 pasos —2 por arriba de la
 *        colmena y 1 bajando a un casillero vacío.
 *
 * Los dos pasos de arriba son una caminata sobre casilleros ocupados: el mismo
 * grafo sobre el que se chequea la regla de la colmena. Por eso esta es la
 * segunda clase que usa `BoardGraphAdapter`, y la razón por la que ese grafo es
 * una clase y no código enterrado adentro de `Board`.
 *
 * Ninguno de los tres pasos es un deslizamiento por el piso: los dos primeros
 * caen arriba de piezas y el tercero es un descenso desde ahí, así que la regla
 * del hueco no aplica en ninguno.
 */
class LadybugMovement : public MovementStrategy {
public:
    /** @copydoc MovementStrategy::moves */
    std::vector<Hex> moves(const Board &board, const Hex &from,
                            const Piece &self) const override;
};
