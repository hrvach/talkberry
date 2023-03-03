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

#define BIT(a) (lfsr >> (a))

uint32_t lfsr = 0xDEADBEEF; /* PRNG seed */
uint32_t get_random_number(void)
{
    uint32_t bit = (BIT(0) ^ BIT(1) ^ BIT(2) ^ BIT(4) ^ BIT(6) ^ BIT(31)) & 1;
    lfsr = (lfsr >> 1) | (bit << 31);
    return lfsr;
}

void phase_synth(MODEL *model, q31_t *prev_phase, q31_t A[])
{
    q31_t Ex[2 * N_SPF + 2] = {0};
    q31_t H[2 * N_SPF + 2];

    /* Shift to Q18, divide by Q9 -> back to Q9 */
    const int step = (FFT_SIZE << Q18BITS) / model->pitch;

    for (int m = 1, i = HALF_FFT_SIZE; m <= model->L; m++, i += step)
    {
        int b = (i >> Q9BITS);
        H[2 * m] = A[2 * b] << 2;
        H[2 * m + 1] = -(A[2 * b + 1] << 2);
    }

    /* Since Wo is in Q28 and phase chosen to be Q24, Wo * 5 is in fact multiplication by 80
       This step updates phase and brings angle back to <-pi, pi> */
    for (*prev_phase += model->Wo * 5; *prev_phase >= PI_Q24;)
        *prev_phase -= TAU_Q24;

    if (!model->voiced)
    {
        /* In unvoiced case, set vectors to random */
        for (int m = 0; m <= 2 * model->L + 1; m++)
            Ex[m] = get_random_number();
    }
    else
    {
        /* prepare the phase angle, convert from Q24 to Q27 */
        q31_t phase = *prev_phase << 3;

        /* Set the initial conditions for our recursion */
        Ex[0] = ONE_IN_Q27; /* cos(0) = 1 */
        Ex[1] = 0;          /* sin(0) = 0 */

        /* Ex[2] = cos(x) -> real part,
           Ex[3] = sin(x) -> imaginary part */
        cordic(phase, &Ex[3], &Ex[2]);

        /* Calculate the common term outside of the loop */
        q63_t _2_Ex2 = 2L * (q63_t)Ex[2];

        for (int m = 2; m <= model->L; m++)
        {
            /* sin(nx) = 2 * sin((n-1)x) * cos(x) - sin((n-2)x) */
            Ex[2 * m + 1] = ((Ex[2 * m - 1] * _2_Ex2) >> Q27BITS) - Ex[2 * m - 3];

            /* cos(nx) = 2 * cos((n-1)x) * cos(x) - cos((n-2)x) */
            Ex[2 * m] = ((Ex[2 * m - 2] * _2_Ex2) >> Q27BITS) - Ex[2 * m - 4];
        }
    }

    /* Apply LPC filter to the excitation sample */
    complex_multiply(H, Ex, model->Af, model->L);
}
