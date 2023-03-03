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
#include "fxpmath.h"

const uint8_t L_LUT[] = {79, 75, 72, 68, 65, 62, 60, 57, 55, 53, 51, 49, 48, 46, 45, 43, 42, 41, 40, 39, 38, 37,
                         36, 35, 34, 33, 33, 32, 31, 30, 30, 29, 29, 28, 27, 27, 26, 26, 25, 25, 25, 24, 24, 23,
                         23, 23, 22, 22, 22, 21, 21, 21, 20, 20, 20, 19, 19, 19, 19, 18, 18, 18, 18, 17, 17, 17,
                         17, 17, 16, 16, 16, 16, 16, 16, 15, 15, 15, 15, 15, 15, 14, 14, 14, 14, 14, 14, 14, 13,
                         13, 13, 13, 13, 13, 13, 13, 12, 12, 12, 12, 12, 12, 12, 12, 12, 11, 11, 11, 11, 11, 11,
                         11, 11, 11, 11, 11, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10};

const q31_t PITCH_LUT[] = {
    81920, 77672, 73843, 70374, 67216, 64330, 61681, 59242, 56988, 54899, 52958, 51150, 49461, 47880, 46397, 45003,
    43691, 42452, 41283, 40175, 39126, 38130, 37184, 36283, 35425, 34606, 33825, 33078, 32363, 31679, 31023, 30394,
    29789, 29208, 28650, 28112, 27594, 27095, 26614, 26149, 25700, 25267, 24848, 24442, 24050, 23670, 23302, 22945,
    22599, 22263, 21937, 21620, 21313, 21014, 20723, 20440, 20165, 19897, 19636, 19382, 19135, 18893, 18658, 18428,
    18204, 17986, 17772, 17564, 17361, 17162, 16967, 16777, 16591, 16410, 16232, 16058, 15888, 15721, 15558, 15398,
    15241, 15087, 14937, 14790, 14645, 14503, 14364, 14228, 14094, 13962, 13833, 13707, 13583, 13461, 13341, 13223,
    13107, 12994, 12882, 12772, 12664, 12558, 12453, 12351, 12250, 12150, 12053, 11956, 11862, 11769, 11677, 11586,
    11498, 11410, 11324, 11239, 11155, 11073, 10991, 10911, 10832, 10755, 10678, 10602, 10528, 10454, 10382, 10310};

const q31_t ENERGY_LUT[] = {512,   606,   717,   848,   1004,  1187,  1405,  1662,  1967,  2327,  2754,
                            3258,  3855,  4561,  5397,  6386,  7556,  8940,  10578, 12517, 14810, 17523,
                            20734, 24533, 29027, 34346, 40638, 48084, 56893, 67317, 79651, 94244};

const q31_t Wo_LUT[] = {
    10541436, 11117920, 11694405, 12270890, 12847375, 13423860, 14000344, 14576829, 15153314, 15729799, 16306283,
    16882768, 17459253, 18035738, 18612222, 19188707, 19765192, 20341677, 20918161, 21494646, 22071131, 22647616,
    23224101, 23800585, 24377070, 24953555, 25530040, 26106524, 26683009, 27259494, 27835979, 28412463, 28988948,
    29565433, 30141918, 30718402, 31294887, 31871372, 32447857, 33024342, 33600826, 34177311, 34753796, 35330281,
    35906765, 36483250, 37059735, 37636220, 38212704, 38789189, 39365674, 39942159, 40518643, 41095128, 41671613,
    42248098, 42824583, 43401067, 43977552, 44554037, 45130522, 45707006, 46283491, 46859976, 47436461, 48012945,
    48589430, 49165915, 49742400, 50318885, 50895369, 51471854, 52048339, 52624824, 53201308, 53777793, 54354278,
    54930763, 55507247, 56083732, 56660217, 57236702, 57813186, 58389671, 58966156, 59542641, 60119126, 60695610,
    61272095, 61848580, 62425065, 63001549, 63578034, 64154519, 64731004, 65307488, 65883973, 66460458, 67036943,
    67613427, 68189912, 68766397, 69342882, 69919367, 70495851, 71072336, 71648821, 72225306, 72801790, 73378275,
    73954760, 74531245, 75107729, 75684214, 76260699, 76837184, 77413668, 77990153, 78566638, 79143123, 79719608,
    80296092, 80872577, 81449062, 82025547, 82602031, 83178516, 83755001};

const q31_t synthesis_window[] = {
    0,          26843546,   53687092,   80530640,   107374184,  134217728,  161061280,  187904832,  214748384,
    241591936,  268435488,  295279040,  322122592,  348966144,  375809696,  402653248,  429496800,  456340352,
    483183904,  510027456,  536870976,  563714496,  590558016,  617401536,  644245056,  671088576,  697932096,
    724775616,  751619136,  778462656,  805306176,  832149696,  858993216,  885836736,  912680256,  939523776,
    966367296,  993210816,  1020054336, 1046897856, 1073741376, 1100584960, 1127428480, 1154272000, 1181115520,
    1207959040, 1234802560, 1261646080, 1288489600, 1315333120, 1342176640, 1369020160, 1395863680, 1422707200,
    1449550720, 1476394240, 1503237760, 1530081280, 1556924800, 1583768320, 1610611840, 1637455360, 1664298880,
    1691142400, 1717985920, 1744829440, 1771672960, 1798516480, 1825360000, 1852203520, 1879047040, 1905890560,
    1932734080, 1959577600, 1986421120, 2013264640, 2040108160, 2066951680, 2093795200, 2120638720, 2147483647,
    2120640128, 2093796608, 2066953088, 2040109568, 2013266048, 1986422528, 1959579008, 1932735488, 1905891968,
    1879048448, 1852204928, 1825361408, 1798517888, 1771674368, 1744830848, 1717987328, 1691143808, 1664300288,
    1637456768, 1610613248, 1583769728, 1556926208, 1530082688, 1503239168, 1476395648, 1449552128, 1422708608,
    1395865088, 1369021568, 1342178048, 1315334528, 1288491008, 1261647488, 1234803968, 1207960448, 1181116928,
    1154273408, 1127429888, 1100586368, 1073742848, 1046899328, 1020055808, 993212288,  966368768,  939525248,
    912681728,  885838208,  858994688,  832151168,  805307648,  778464128,  751620608,  724777088,  697933568,
    671090048,  644246528,  617403008,  590559488,  563715968,  536872448,  510028896,  483185344,  456341792,
    429498240,  402654688,  375811136,  348967584,  322124032,  295280480,  268436928,  241593376,  214749824,
    187906272,  161062720,  134219168,  107375624,  80532080,   53688536,   26844990};

const int32_t cordic_atan_table[] = {
    0x06487ed5, 0x03b58ce0, 0x01f5b75f, 0x00feadd4, 0x007fd56e, 0x003ffaab, 0x001fff55,
    0x000fffea, 0x0007fffd, 0x0003ffff, 0x0001ffff, 0x0000ffff, 0x00007fff, 0x00003fff,
    0x00001fff, 0x00000fff, 0x000007ff, 0x000003ff, 0x000001ff, 0x000000ff, 0x0000007f,
    0x0000003f, 0x0000001f, 0x0000000f, 0x00000007, 0x00000003, 0x00000001, 0x00000001,
};

const q31_t codebook[] = {
    23718230,  26353590,  28988948,  31624308,  34259668,  36895028,  39530384,  42165744,  44801100,  47436460,
    50071824,  52707180,  55342540,  57977896,  60613256,  63248616,  34259668,  36895028,  39530384,  42165744,
    44801100,  47436460,  50071824,  52707180,  55342540,  57977896,  60613256,  63248616,  65883976,  68519336,
    71154696,  73790056,  52707180,  57977896,  63248616,  68519336,  73790056,  79060768,  84331488,  89602200,
    94872920,  100143648, 105414360, 110685080, 115955792, 121226512, 126497232, 131767952, 73790056,  84331488,
    94872920,  105414360, 115955792, 126497232, 137038672, 147580112, 158121536, 168662976, 179204400, 189745840,
    200287296, 210828720, 221370160, 231911584, 100143648, 110685080, 121226512, 131767952, 142309392, 152850832,
    163392256, 173933696, 184475120, 195016576, 205558000, 216099440, 226640864, 237182304, 247723760, 258265184,
    115955792, 126497232, 137038672, 147580112, 158121536, 168662976, 179204400, 189745840, 200287296, 210828720,
    221370160, 231911584, 242453024, 252994464, 263535904, 274077344, 158121536, 168662976, 179204400, 189745840,
    200287296, 210828720, 221370160, 231911584, 242453024, 252994464, 263535904, 274077344, 284618784, 295160224,
    305701664, 316243072, 242453024, 252994464, 263535904, 274077344, 284618784, 295160224, 305701664, 316243072,
    263535904, 274077344, 284618784, 295160224, 305701664, 316243072, 326784512, 337325952, 305701664, 326784512,
    347867392, 368950240};

const int lsp_bits[] = {4, 4, 4, 4, 4, 4, 4, 3, 3, 2};
const int lsp_masks[] = {15, 15, 15, 15, 15, 15, 15, 7, 7, 3};
const int lsp_offsets[] = {0, 16, 32, 48, 64, 80, 96, 112, 120, 128};