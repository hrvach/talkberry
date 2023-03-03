/*
Copyright (c) 2023 Hrvoje Cavrak, David Rowe

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  See the file LICENSE included with this distribution for more
  information.
*/

#include "codec2.h"
#include "defines.h"
#include "fxpmath.h"

static MODEL unvoiced_model = {.Wo = TAU_Q28 / P_MAX, .pitch = MAX_PITCH, .L = MAX_L};

void interpolate_Wo(MODEL *frame, MODEL *prev, MODEL *current, int index)
{
    /* If neither previous or current are voiced, neither is the target frame */
    frame->voiced &= (prev->voiced | current->voiced);

    if (!frame->voiced)
    {
        *frame = unvoiced_model;
        return;
    }

    switch ((prev->voiced << 1) | current->voiced)
    {
    /* Current one is voiced - fill using current value */
    case 1:
        *frame = *current;
        break;

    /* Previous one was voiced - fill using previous value */
    case 2:
        *frame = *prev;
        break;

    /* Both were voiced, interpolate */
    case 3:
        frame->Wo = ((3 - index) * prev->Wo + (index + 1) * current->Wo) >> 2;
        frame->pitch = (TAU_Q28 / (frame->Wo >> 9)); /* Wo is in Q28 now, we need the result in Q9 */
        frame->L = (PI_Q28 / (frame->Wo));           /* Both PI and Wo are in Q28, result is just L */
        break;
    }
}

void interpolate_energy(MODEL *new, MODEL *prev, MODEL *current, int index)
{
    /* Check for equality first to avoid expensive sqrt if not needed */
    if (prev->energy == current->energy)
        new->energy = current->energy;

    else
        new->energy = (3 - index) * (prev->energy >> 2) + ((index + 1) * current->energy >> 2);
}

/*
    Equation is: (1 - weight) * prev[i] + weight * current[i];
    Weights relate to indexes and we can implement it using fast bit shifts.
    N goes from 0-2, weights can be 0.75 for n = 0, 0.5 for n = 1 or 0.25 for n = 2
*/
void interpolate_lsp(q31_t interpolated[], q31_t prev[], q31_t current[], q31_t n)
{
    for (int i = 0; i < LPC_ORD; i++)
        interpolated[i] = (3 - n) * (prev[i] >> 2) + (n + 1) * (current[i] >> 2);
}
