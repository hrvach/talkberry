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
#include "fft_tables.h"
#include "fxpmath.h"

/* Helper function to calculate the linear prediction polynomial coefficients. */
void lsp_to_polynomial(const q31_t coeffs[], q31_t poly[])
{
    poly[0] = ONE_IN_Q23;
    poly[1] = -coeffs[0] * 2; /* Coeffs also in Q23 */

    for (int i = 2; i <= LPC_ORD / 2; i++)
    {
        q31_t b = 2 * (-coeffs[2 * i - 2]);
        poly[i] = MUL_SHIFT(b, poly[i - 1], Q23BITS) + 2 * poly[i - 2];

        for (int j = i - 1; j > 1; j--)
            poly[j] += MUL_SHIFT(b, poly[j - 1], Q23BITS) + poly[j - 2];

        poly[1] += b;
    }
}

/* Convert line spectral frequencies to line spectral pairs, Q27 -> Q23 */
void lsf_to_lsp(q31_t lsf[], q31_t lsp[])
{
    for (q31_t j = 0, sin = 0, cos = 0; j < LPC_ORD; j++)
    {
        cordic(lsf[j], &sin, &cos);
        lsp[j] = cos >> 4;
    }
}

/* Convert line spectral pairs to linear prediction coefficients */
void lsp_to_lpc(q31_t lsp[], q31_t lpc[])
{
    q31_t p[LPC_ORD + 1], q[LPC_ORD + 1];

    /* Get the coefficients for the two polynomials */
    lsp_to_polynomial(&lsp[0], p);
    lsp_to_polynomial(&lsp[1], q);

    for (int i = LPC_ORD / 2; i > 0; i--)
    {
        p[i] += p[i - 1];
        q[i] -= q[i - 1];
    }

    lpc[0] = ONE_IN_Q23;

    /* Use the polynomial coefficients to calculate the LPC */
    for (int i = 1, j = LPC_ORD; i <= LPC_ORD / 2; i++, j--)
    {
        lpc[i] = (p[i] + q[i]) >> 1;
        lpc[j] = (p[i] - q[i]) >> 1;
    }
}

void check_lsp_order(q31_t lsp[])
{
    /* LSP coeffs are expected to be in proper order. If not, fix it */
    for (int i = 1; i < LPC_ORD; i++)
    {
        if (lsp[i] < lsp[i - 1])
        {
            q31_t old_lsp_value = lsp[i - 1];
            lsp[i - 1] = lsp[i] - POINT_ONE_IN_Q27;
            lsp[i] = old_lsp_value + POINT_ONE_IN_Q27;

            /* If order was not correct, fix and start over */
            i = 1;
        }
    }
}

static void lpc_post_filter(uint64_t Pw[], q31_t Aw[])
{
    Pw[0] = 0;

    for (int i = 0; i < (FFT_SIZE / 2); i++)
    {
        uint64_t re2 = (uint64_t)((q63_t)Aw[2 * i] * (q63_t)Aw[2 * i]);
        uint64_t im2 = (uint64_t)((q63_t)Aw[2 * i + 1] * (q63_t)Aw[2 * i + 1]);

        uint32_t mag_inv = SAT((re2 + im2) >> Q9BITS);

        Pw[i] = (uint32_t)(ONE_IN_Q32 / mag_inv);

        /* Simple noise filter threshold  */
        if (Pw[i] < ONE_IN_Q12)
            Pw[i] = 0;
        else
            Pw[i] -= ONE_IN_Q12;
    }
}

/* Convert LPC indexes to frequency domain amplitudes */
void lpc_to_amplitudes(arm_rfft_instance_q31 *arm_fft, q31_t ak[], MODEL *model, q31_t E, q31_t Aw[], int e_index)
{
    uint64_t Pw[FFT_SIZE / 2 + 1] = {0};
    q31_t lpc_coeffs[FFT_SIZE] = {0};
    uint64_t bin_power, Am;

    for (int i = 0; i <= LPC_ORD; i++)
        lpc_coeffs[i] = ak[i];

    /* Apply FFT transform on LPC coefficients */
    arm_rfft_q31(arm_fft, lpc_coeffs, Aw);
    lpc_post_filter(Pw, Aw);

    int start = (model->Wo / TAU_Q11);
    int step = 2 * start;

    for (int m = 1, i = (start); m <= model->L; m++, i += step)
    {
        /* Calculate band limits */
        int am = (i + ONE_HALF_IN_Q9) >> Q9BITS;
        int bm = (i + step + ONE_HALF_IN_Q9) >> Q9BITS;

        if (bm > FFT_SIZE / 2)
            bm = FFT_SIZE / 2;

        bin_power = 0;
        for (int j = am; j < bm; j++)
            bin_power += Pw[j];

        Am = MUL_SHIFT(E, bin_power, 16);

        /* Am *= 0.75 */
        if (Am > model->A[m])
            Am = (Am >> 1) + (Am >> 2);

        /* Am *= 1.5 */
        if (Am < model->A[m])
            Am = Am + (Am >> 1);

        model->A[m] = Am;
    }
}

void decode_lsps_scalar(q31_t lsp[], int indexes[])
{
    for (int i = 0; i < LPC_ORD; i++)
        lsp[i] = codebook[lsp_offsets[i] + indexes[i]];
}

void bw_expand_lsps(q31_t lsp[])
{
    for (int i = 1; i < LPC_ORD; i++)
    {
        q31_t THRESH = (i >= 4) ? MIN_SEP_HIGH : MIN_SEP_LOW;

        if ((lsp[i] - lsp[i - 1]) < THRESH)
            lsp[i] = ADD(lsp[i - 1], THRESH);
    }
}

/* Improve results for low-pitched male speakers */
void apply_lpc_correction(MODEL *model)
{
    /* Wo is in Q28, compared to PI * 150 / 4000 in Q28 == 31624307 */
    if (model->Wo < PITCH_53_IN_Q28)
    {
        /* Wo is multiplied by 0.032 which is very close to 32/1024,
         * so we can achieve the same by right shifting 5 places */
        model->A[1] >>= 5;
    }
}
