#ifndef FXPMATH__H
#define FXPMATH__H

#include <stdint.h>

/* Fixed point types */
typedef int16_t q15_t;
typedef int32_t q31_t;
typedef int64_t q63_t;

/* Convenient fixed point definitions */
#define Q9BITS 9
#define Q15BITS 15
#define Q18BITS 18
#define Q23BITS 23
#define Q27BITS 27
#define Q31BITS 31
#define Q32BITS 32

#define Q9 512
#define Q15 32768
#define Q23 8388608
#define Q24 16777216
#define Q26 67108864
#define Q27 134217728
#define Q28 268435456
#define Q31 2147483648

/* Various constants in different representations */
#define ONE_IN_Q12 0x00000FFF
#define ONE_IN_Q23 0x007FFFFF
#define ONE_IN_Q27 0x07FFFFFF
#define ONE_IN_Q32 0xFFFFFFFF

#define ONE_HALF_IN_Q9 256
#define POINT_ONE_IN_Q27 (q31_t)0xcccccd

#define PI_Q24 52707179  /* Q24 * pi */
#define PI_Q28 843314857 /* Q28 * pi */

#define TAU_Q11 12868
#define TAU_Q24 105414357
#define TAU_Q26 421657428  /* Q26 * 2 pi */
#define TAU_Q28 1686629713 /* Q28 * 2 pi */

/* Casting macros */
#define I64(a) ((int64_t)a)
#define I32(a) ((int32_t)a)

/* Basic fixed-point operations */
#define ABS(a) ((a > 0) ? a : -a)
#define ADD(a, b) (SAT(I64(a) + I64(b)))
#define SUB(a, b) (SAT(I64(a) - I64(b)))

#define MUL_SHIFT(a, b, s) (SAT((q31_t)(I64(a) * I64(b) >> s)))
#define MUL_Q31(a, b) ((q31_t)SAT((I64(a) * I64(b) >> Q31BITS)))

/* Saturation / clamping */
#define SAT_PLUS(a, lim) (a > lim - 1 ? lim - 1 : a)
#define SAT_MINUS(a, lim) (a < -lim ? -lim : a)

#define SAT(a) (SAT_PLUS(SAT_MINUS(I64(a), Q31), Q31))
#define SAT15(a) (SAT_PLUS(SAT_MINUS(I32(a), Q15), Q15))

/* Enables to make MUL() behave differently with 2 and 3 arguments */
#define GET_MACRO(_1, _2, _3, NAME, ...) NAME
#define MUL(...) GET_MACRO(__VA_ARGS__, MUL_SHIFT, MUL_Q31)(__VA_ARGS__)

#endif
