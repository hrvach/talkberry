/*
Copyright (c) 2023 Hrvoje Cavrak, David Rowe

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  See the file LICENSE included with this distribution for more
  information.
*/

#ifndef __DEFINES__
#define __DEFINES__
#include "fxpmath.h"

#define N_SPF 80
#define FFT_SIZE 512
#define HALF_FFT_SIZE 256
#define LPC_ORD 10

#define P_MAX 160

#define MIN_SEP_LOW 5270718   /* 50 x PI / 4000 in Q27 */
#define MIN_SEP_HIGH 10541436 /* 100 x PI / 4000 in Q27 */
#define PITCH_53_IN_Q28 31624307

#define LIMIT_THRESH 30000
#define NUM_FRAMES 4

#define MAX_PITCH 81920
#define MAX_L 79

    /* Structure to hold received params for 4 frames */
    typedef struct
    {
        int voiced[4];             /* Voicing information for each of the 4 packets (4 bit) */
        int e_index;               /* Index in the energy lookup table (5 bits) */
        int Wo_index;              /* Pitch index in the pitch lookup table (7 bits) */
        int lsp_indexes[LPC_ORD];  /* Indexes for looking up LSPs in the codebook (36 bits) */
    } codec2_pkt;

    /* Structure to hold model parameters for one frame */
    typedef struct
    {
        int64_t count;             /* Frame number, currently unused */
        q31_t Wo;                  /* Fixed point representation of fundamental frequency in Q28 */
        q31_t pitch;               /* Fixed point representation of pitch in Q9 */
        q31_t energy;              /* Frame energy */
        int L;                     /* Harmonics count */
        q31_t A[N_SPF + 1];        /* Harmonics amplitude */
        q31_t Af[2 * N_SPF + 2];   /* sin+cos for waveform synthesis */
        q31_t H[N_SPF + 1];        /* Magnitudes for waveform synthesis */
        int voiced;                /* One if this frame is voiced */
    } MODEL;

#endif
