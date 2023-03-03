/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_common_tables.h
 * Description:  Extern declaration for common tables
 *
 * @version  V1.10.0
 * @date     08 July 2021
 *
 * Target Processor: Cortex-M and Cortex-A cores
 * -------------------------------------------------------------------- */
/*
 * Copyright (C) 2010-2021 ARM Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _ARM_COMMON_TABLES_H
#define _ARM_COMMON_TABLES_H

#include "arm_math_types.h"
#include "dsp/fast_math_functions.h"

#ifdef   __cplusplus
extern "C"
{
#endif

  /* Twiddle coeffs Q31 */

    extern const q31_t twiddleCoef_16_q31[24];
    extern const q31_t twiddleCoef_32_q31[48];
    extern const q31_t twiddleCoef_64_q31[96];
    extern const q31_t twiddleCoef_128_q31[192];
    extern const q31_t twiddleCoef_256_q31[384];
    extern const q31_t twiddleCoef_512_q31[768];


  /* fixed-point bit reversal tables */

    #define ARMBITREVINDEXTABLE_FIXED_16_TABLE_LENGTH ((uint16_t)12)
    extern const uint16_t armBitRevIndexTable_fixed_16[ARMBITREVINDEXTABLE_FIXED_16_TABLE_LENGTH];

    #define ARMBITREVINDEXTABLE_FIXED_32_TABLE_LENGTH ((uint16_t)24)
    extern const uint16_t armBitRevIndexTable_fixed_32[ARMBITREVINDEXTABLE_FIXED_32_TABLE_LENGTH];

    #define ARMBITREVINDEXTABLE_FIXED_64_TABLE_LENGTH ((uint16_t)56)
    extern const uint16_t armBitRevIndexTable_fixed_64[ARMBITREVINDEXTABLE_FIXED_64_TABLE_LENGTH];

    #define ARMBITREVINDEXTABLE_FIXED_128_TABLE_LENGTH ((uint16_t)112)
    extern const uint16_t armBitRevIndexTable_fixed_128[ARMBITREVINDEXTABLE_FIXED_128_TABLE_LENGTH];

    #define ARMBITREVINDEXTABLE_FIXED_256_TABLE_LENGTH ((uint16_t)240)
    extern const uint16_t armBitRevIndexTable_fixed_256[ARMBITREVINDEXTABLE_FIXED_256_TABLE_LENGTH];

    #define ARMBITREVINDEXTABLE_FIXED_512_TABLE_LENGTH ((uint16_t)480)
    extern const uint16_t armBitRevIndexTable_fixed_512[ARMBITREVINDEXTABLE_FIXED_512_TABLE_LENGTH];

    extern const q31_t realCoefAQ31[8192];
    extern const q31_t realCoefBQ31[8192];

#ifdef   __cplusplus
}
#endif

#endif /*  ARM_COMMON_TABLES_H */

