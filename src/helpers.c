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

void complex_multiply(q31_t *a, q31_t *b, q31_t *dst, int len)
{
    for (int i = 0; i < len; i++)
    {
        q31_t ar = a[2 * i];     /* Real part of a[i]      */
        q31_t ai = a[2 * i + 1]; /* Imaginary part of a[i] */
        q31_t br = b[2 * i];     /* Real part of b[i]      */
        q31_t bi = b[2 * i + 1]; /* Imaginary part of b[i] */

        dst[2 * i] = SUB(MUL(ar, br), MUL(ai, bi));     /* Real part of a[i] * b[i] */
        dst[2 * i + 1] = ADD(MUL(ar, bi), MUL(ai, br)); /* Imaginary part of a[i] * b[i] */
    }
}

void shift_left(q31_t src[], q31_t dst[], int len)
{
    for (int i = 0; i < len; i++)
        dst[i] = src[i];
}

void cordic(int32_t theta, q31_t *sin, q31_t *cos)
{
    const int HALF_PI = 0xc90fdaa;
    int x = 0x4dba76d;
    int y = 0;
    int z = theta;

    if (theta > HALF_PI || theta < -HALF_PI)
    {
        if (theta < 0)
            z = theta + 2 * HALF_PI;
        else
            z = theta - 2 * HALF_PI;

        x = -x;
    }

    for (int i = 0; i < 28; ++i)
    {
        int d = z >> 31;

        int tx = x - (((y >> i) ^ d) - d);
        y = y + (((x >> i) ^ d) - d);
        z = z - ((cordic_atan_table[i] ^ d) - d);
        x = tx;
    }

    *cos = x;
    *sin = y;
}

int decode_gray(int num)
{
    num ^= num >> 8;
    num ^= num >> 4;
    num ^= num >> 2;
    num ^= num >> 1;
    return num;
}

void unpack(unsigned char *input, codec2_pkt *pkt, int is_odd)
{
    /*             6        5        4        3        2        1        0
      Even packet |VVVVWWWW|WWWEEEEE|LLLLLLLL|LLLLLLLL|LLLLLLLL|LLLLLLLL|LLLL____|
       Odd packet |____VVVV|WWWWWWWE|EEEELLLL|LLLLLLLL|LLLLLLLL|LLLLLLLL|LLLLLLLL|
    */
    uint64_t in = 0;

    for (int i = 0; i < 7; i++)
        in = (in << 8) + input[i];

    if (is_odd)
        in <<= 4;

    /* 4 bits for voicings */
    pkt->voiced[3] = decode_gray((in >> 52) & 1);
    pkt->voiced[2] = decode_gray((in >> 53) & 1);
    pkt->voiced[1] = decode_gray((in >> 54) & 1);
    pkt->voiced[0] = decode_gray((in >> 55) & 1);

    /* 7 bits for Wo index */
    pkt->Wo_index = decode_gray((in >> 45) & 0x7f);

    /* 5 bits for E index */
    pkt->e_index = decode_gray((in >> 40) & 0x1f);

    /* 36 bits for LSP indexes */
    uint64_t lsp = (in >> 4);

    for (int i = LPC_ORD - 1; i >= 0; i--)
    {
        pkt->lsp_indexes[i] = decode_gray(lsp & lsp_masks[i]);
        lsp = lsp >> lsp_bits[i];
    }
}

q63_t estimate_magnitude(q31_t re, q31_t im)
{
    /* Refined alpha max plus beta min algorithm to approximate magnitude without sqrt
       alpha0 = 1, beta0 = 5/32, alpha1 = 27/32, beta2 = 71/128, max error = 1.22%
    */

    /* Find absolute values of real and imaginary components */
    re = ABS(re);
    im = ABS(im);

    /* Determine which is larger */
    q31_t larger = re > im ? re : im;
    q31_t smaller = re < im ? re : im;

    /* Calculate zN as alphaN * larger + betaN * smaller */
    q63_t z0 = larger + (5 * smaller >> 5);
    q63_t z1 = ((27 * larger) >> 5) + ((71 * smaller) >> 7);

    /* Approximation is done by returning the larger of the two */
    return (z0 > z1) ? z0 : z1;
}
