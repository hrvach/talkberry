/*
Copyright (c) 2023 Hrvoje Cavrak, David Rowe

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  See the file LICENSE included with this distribution for more
  information.
*/

#include "defines.h"
#include "dsp/transform_functions.h"
#include "fxpmath.h"

//////////////////////////////// PUBLIC ////////////////////////////////////////////////

void codec2_init();
void codec2_decode(short speech[], unsigned char *bits);

//////////////////////////////// PRIVATE ///////////////////////////////////////////////

/* Sine */
int synthesise(arm_rfft_instance_q31 *fft, q31_t Sn_[], MODEL *model, const q31_t Pn[]);

/* Phase */
void phase_synth(MODEL *model, q31_t *prev_phase, q31_t A[]);

/* Interpolate */
void interpolate_energy(MODEL *new, MODEL *prev, MODEL *current, int index);
void interpolate_Wo(MODEL *interpolate, MODEL *prev, MODEL *next, int index);
void interpolate_lsp(q31_t interp[], q31_t prev[], q31_t next[], q31_t index);

/* Quantise */
void lpc_to_amplitudes(arm_rfft_instance_q31 *arm_fft, q31_t ak[], MODEL *model, q31_t E, q31_t Aw[], int e_index);
void lsf_to_lsp(q31_t lsf[], q31_t lsp[]);
void lsp_to_lpc(q31_t lsp[], q31_t lpc[]);
void bw_expand_lsps(q31_t lsp[]);
void check_lsp_order(q31_t lsp[]);
void decode_lsps_scalar(q31_t lsp[], int indexes[]);
void apply_lpc_correction(MODEL *model);

/* Main */
void unpack_and_decode(MODEL model[], codec2_pkt *pkt, q31_t received_lsf[], unsigned char *bits);
void ear_protection(q31_t sample[], int max_amplitude);

/* Helpers */
int decode_gray(int num);
void unpack(unsigned char *input, codec2_pkt *pkt, int is_odd);
void cordic(int32_t theta, q31_t *sin, q31_t *cos);
void shift_left(q31_t src[], q31_t dst[], int len);
void complex_multiply(q31_t a[], q31_t b[], q31_t dst[], int len);
q63_t estimate_magnitude(q31_t re, q31_t im);

/* Lookup tables */
extern const int32_t cordic_atan_table[];
extern const q31_t synthesis_window[];
extern const q31_t Wo_LUT[];
extern const q31_t ENERGY_LUT[];
extern const q31_t PITCH_LUT[];
extern const uint8_t L_LUT[];

extern const q31_t codebook[];
extern const int lsp_bits[];
extern const int lsp_masks[];
extern const int lsp_offsets[];
