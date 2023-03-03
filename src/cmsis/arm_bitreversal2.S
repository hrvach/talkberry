;/* ----------------------------------------------------------------------
; * Project:      CMSIS DSP Library
; * Title:        arm_bitreversal2.S
; * Description:  arm_bitreversal_32 function done in assembly for maximum speed.
; *               Called after doing an fft to reorder the output.
; *               The function is loop unrolled by 2. arm_bitreversal_16 as well.
; *
; * $Date:        18. March 2019
; * $Revision:    V1.5.2
; *
; * Target Processor: Cortex-M cores
; * -------------------------------------------------------------------- */
;/*
; * Copyright (C) 2010-2019 ARM Limited or its affiliates. All rights reserved.
; *
; * SPDX-License-Identifier: Apache-2.0
; *
; * Licensed under the Apache License, Version 2.0 (the License); you may
; * not use this file except in compliance with the License.
; * You may obtain a copy of the License at
; *
; * www.apache.org/licenses/LICENSE-2.0
; *
; * Unless required by applicable law or agreed to in writing, software
; * distributed under the License is distributed on an AS IS BASIS, WITHOUT
; * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
; * See the License for the specific language governing permissions and
; * limitations under the License.
; */

    #define THUMB .thumb
    #define CODESECT .section .text
    #define EXPORT .global
    #define PROC :
    #define LABEL :
    #define ENDP
    #define END

    .syntax unified

	CODESECT
	THUMB

;/**
;  @brief         In-place bit reversal function.
;  @param[in,out] pSrc        points to the in-place buffer of unknown 32-bit data type
;  @param[in]     bitRevLen   bit reversal table length
;  @param[in]     pBitRevTab  points to bit reversal table
;  @return        none
; */
	EXPORT arm_bitreversal_32
	EXPORT arm_bitreversal_16

	.type   arm_bitreversal_16, %function
	.type   arm_bitreversal_32, %function

arm_bitreversal_32 PROC
	ADDS     r3,r1,#1
	PUSH     {r4-r6}
	ADDS     r1,r2,#0
	LSRS     r3,r3,#1
arm_bitreversal_32_0 LABEL
	LDRH     r2,[r1,#2]
	LDRH     r6,[r1,#0]
	ADD      r2,r0,r2
	ADD      r6,r0,r6
	LDR      r5,[r2,#0]
	LDR      r4,[r6,#0]
	STR      r5,[r6,#0]
	STR      r4,[r2,#0]
	LDR      r5,[r2,#4]
	LDR      r4,[r6,#4]
	STR      r5,[r6,#4]
	STR      r4,[r2,#4]
	ADDS     r1,r1,#4
	SUBS     r3,r3,#1
	BNE      arm_bitreversal_32_0
	POP      {r4-r6}
	BX       lr
	ENDP

arm_bitreversal_16 PROC
	ADDS     r3,r1,#1
	PUSH     {r4-r6}
	ADDS     r1,r2,#0
	LSRS     r3,r3,#1
arm_bitreversal_16_0 LABEL
	LDRH     r2,[r1,#2]
	LDRH     r6,[r1,#0]
    LSRS     r2,r2,#1
    LSRS     r6,r6,#1
	ADD      r2,r0,r2
	ADD      r6,r0,r6
	LDR      r5,[r2,#0]
	LDR      r4,[r6,#0]
	STR      r5,[r6,#0]
	STR      r4,[r2,#0]
	ADDS     r1,r1,#4
	SUBS     r3,r3,#1
	BNE      arm_bitreversal_16_0
	POP      {r4-r6}
	BX       lr
	ENDP


	END
