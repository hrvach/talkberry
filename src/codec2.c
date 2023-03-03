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

/* FFT instances from the ARM CMSIS FFT routines */
arm_rfft_instance_q31 fft;
arm_rfft_instance_q31 inverse_fft;

/* Initialize the previous model struct with some defaults */
MODEL prev_model = {.Wo = TAU_Q28 / P_MAX, .pitch = MAX_PITCH, .L = MAX_L, .energy = ONE_IN_Q12};

q31_t Sn[4 * N_SPF] = {0}; /* Speech samples in time domain */
q31_t prev_phase = 0;      /* Previous phase value */
q31_t prev_lsfs[LPC_ORD];  /* Previous line spectral frequencies received */

void codec2_init()
{
    /* Initialize FFT structures */
    arm_rfft_init_q31(&fft, FFT_SIZE, 0, 1);
    arm_rfft_init_q31(&inverse_fft, FFT_SIZE, 1, 1);

    /* Set the starting LSPS values so there is no initial "click" in the decoding */
    for (int i = 0; i < LPC_ORD; i++)
        prev_lsfs[i] = i * (TAU_Q26 / (LPC_ORD + 1));
}

void ear_protection(q31_t sample[], int max_amplitude)
{
    if (max_amplitude > LIMIT_THRESH)
    {
        int scaling_factor = (LIMIT_THRESH * LIMIT_THRESH) / max_amplitude;
        scaling_factor = (scaling_factor << Q15BITS) / max_amplitude;

        for (int i = 0; i < N_SPF; i++)
            sample[i] = (sample[i] * scaling_factor) >> Q15BITS;
    }
}

void unpack_and_decode(MODEL model[], codec2_pkt *pkt, q31_t received_lsf[], unsigned char *bits)
{
    /* Interpret and decode the incoming packet */
    unpack(bits, pkt, 0);

    /* Decode voicings and update models */
    for (int i = 0; i < 4; i++)
        model[i].voiced = pkt->voiced[i];

    /* Decode fundamental frequency, in Q28 */
    model[3].Wo = Wo_LUT[pkt->Wo_index];

    /* Decode and store period for convenience, in Q9 */
    model[3].pitch = PITCH_LUT[pkt->Wo_index];

    /* Decode L, in Q0 */
    model[3].L = L_LUT[pkt->Wo_index];

    /* Decode Energy, in Q15 */
    model[3].energy = ENERGY_LUT[pkt->e_index];

    /* Decode received line spectral frequencies */
    decode_lsps_scalar(received_lsf, pkt->lsp_indexes);

    /* Verify they are in the proper order, correct if not */
    check_lsp_order(received_lsf);

    /* Prevents the distance between two LSPs from getting too small */
    bw_expand_lsps(received_lsf);
}

void interpolate(MODEL model[], q31_t received_lsf[], q31_t lsf[][LPC_ORD])
{
    /* We have the values for packet #4, for packets 1-3 we interpolate the values */
    for (int i = 0; i < 3; i++)
    {
        interpolate_lsp(&lsf[i][0], prev_lsfs, received_lsf, i);
        interpolate_Wo(&model[i], &prev_model, &model[3], i);
        interpolate_energy(&model[i], &prev_model, &model[3], i);
    }
}

void codec2_decode(short speech[], unsigned char *bits)
{
    MODEL model[NUM_FRAMES]; /* Parameters for each of the 4 frames */
    codec2_pkt pkt;          /* Structure describing the 52-bit packet itself */

    q31_t lsf[NUM_FRAMES][LPC_ORD] = {0}; /* Line spectral frequencies */
    q31_t lsp[NUM_FRAMES][LPC_ORD];       /* Line spectral pairs */
    q31_t lpc[NUM_FRAMES][LPC_ORD + 1];   /* Linear prediction coefficients */

    /* Move data received to appropriate memory structs */
    unpack_and_decode(model, &pkt, &lsf[3][0], bits);

    /* We have all values for frame 4, the rest we interpolate */
    interpolate(model, &lsf[3][0], lsf);

    q31_t amplitudes[FFT_SIZE * 2] = {0};

    /* Process each frame, from initial values down to time domain samples */
    for (int i = 0; i < NUM_FRAMES; i++)
    {
        /* Line spectral frequencies to line spectral pairs, Q27 -> Q23 */
        lsf_to_lsp(&lsf[i][0], &lsp[i][0]);

        /* Convert line spectral pairs to linear prediction coefficients */
        lsp_to_lpc(&lsp[i][0], &lpc[i][0]);

        /* Convert LPC indexes to frequency domain amplitudes */
        lpc_to_amplitudes(&fft, &lpc[i][0], &model[i], model[i].energy, amplitudes, pkt.e_index);

        /* Correct LPC coefficient */
        apply_lpc_correction(&model[i]);

        /* Generate excitation and apply filter with the LPC coefficients */
        phase_synth(&model[i], &prev_phase, amplitudes);

        /* Calculate real and imag parts of the freq domain spectrum, call inverse FFT to get time domain */
        int max_amplitude = synthesise(&inverse_fft, Sn, &model[i], synthesis_window);

        /* Limit output energy to protect the listener's eardrums */
        ear_protection(Sn, max_amplitude);

        /* Update the output buffer, applying a simple low-pass filter */
        short *speech_target = &speech[N_SPF * i];

        for (int k = 0; k < N_SPF; k++)
            speech_target[k] = SAT15(Sn[k] + (Sn[k + 1] >> 5));
    }

    /* Keep track of previous values so we can do frame value interpolation */
    prev_model = model[3];

    for (int i = 0; i < LPC_ORD; i++)
        prev_lsfs[i] = lsf[3][i];
}
