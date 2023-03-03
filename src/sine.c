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
#include "dsp/transform_functions.h"
#include "fxpmath.h"

void freq_domain_calc(q31_t Sw_[], MODEL *model)
{
    /* Shift to Q18, divide by Q9 -> back to Q9 */
    const int step = (FFT_SIZE << Q18BITS) / model->pitch;

    for (int j = 1, i = ONE_HALF_IN_Q9 + step; j <= model->L; j++, i += step)
    {
        int k = (i >> Q9BITS);

        /* Prevent index exceeding the array maximum */
        if (k >= FFT_SIZE >> 1)
            k = (FFT_SIZE >> 1) - 1;

        /* Approximate the magnitude and use {re, im} / magnitude to get the trig values */
        int64_t magnitude = estimate_magnitude(model->Af[2 * j], model->Af[2 * j + 1]) << 1;

        /* real Sw[k] = A[j] * cos(phi) */
        int64_t real = (model->A[j] * ((int64_t)model->Af[2 * j])) / magnitude;

        /* imag Sw[k] = A[j] * sin(phi) */
        int64_t imag = (model->A[j] * ((int64_t)model->Af[2 * j + 1])) / magnitude;

        Sw_[2 * k] = real;
        Sw_[2 * k + 1] = imag;

        /* Using the frequency domain symmetry */
        Sw_[2 * FFT_SIZE - 2 * k] = real;
        Sw_[2 * FFT_SIZE - 2 * k + 1] = -imag;
    }
}

int synthesise(arm_rfft_instance_q31 *fft, q31_t Sn_[], MODEL *model, const q31_t Pn[])
{
    /* Frequency domain array */
    q31_t Sw_[FFT_SIZE * 2 + 1] = {0};

    /* Time domain array */
    q31_t sw_[FFT_SIZE + 2];

    /* Loop counters, indexes, peak amplitude values */
    int i, j, max_amplitude, abs_value;

    /* Shift the existing samples so we can add the new one */
    shift_left(&Sn_[N_SPF], Sn_, N_SPF - 1);
    Sn_[N_SPF - 1] = 0;

    /* Construct the frequency domain from amplitudes and phases stored in frame's model */
    freq_domain_calc(Sw_, model);

    /* Perform inverse FFT to transform the frequency domain back to time domain */
    arm_rfft_q31(fft, Sw_, sw_);

    /* Multiply with the synthesis window and copy the samples, while we're
       at it, find the max_amplitude we'll use later for ear_protection */
    for (i = 0, max_amplitude = 0; i < (N_SPF - 1); i++)
    {
        Sn_[i] += MUL_SHIFT(sw_[FFT_SIZE - N_SPF + 1 + i], Pn[i], Q32BITS);
        abs_value = ABS(Sn_[i]);

        if (abs_value > max_amplitude)
            max_amplitude = abs_value;
    }

    for (i = N_SPF - 1, j = 0; i < (2 * N_SPF); i++, j++)
        Sn_[i] = MUL_SHIFT(sw_[j], Pn[i], Q32BITS);

    return max_amplitude;
}
