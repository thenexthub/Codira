/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

/*---
desc: #Issue 31487 large number of case statement
tags: []
---*/

enum E {
  E0,
  E1,
  E2,
  E3,
  E4,
  E5,
  E6,
  E7,
  E8,
  E9,
  E10,
  E11,
  E12,
  E13,
  E14,
  E15,
  E16,
  E17,
  E18,
  E19,
  E20,
  E21,
  E22,
  E23,
  E24,
  E25,
  E26,
  E27,
  E28,
  E29,
  E30,
  E31,
  E32,
  E33,
  E34,
  E35,
  E36,
  E37,
  E38,
  E39,
  E40,
  E41,
  E42,
  E43,
  E44,
  E45,
  E46,
  E47,
  E48,
  E49,
  E50,
  E51,
  E52,
  E53,
  E54,
  E55,
  E56,
  E57,
  E58,
  E59,
  E60,
  E61,
  E62,
  E63,
  E64,
  E65,
  E66,
  E67,
  E68,
  E69,
  E70,
  E71,
  E72,
  E73,
  E74,
  E75,
  E76,
  E77,
  E78,
  E79,
  E80,
  E81,
  E82,
  E83,
  E84,
  E85,
  E86,
  E87,
  E88,
  E89,
  E90,
  E91,
  E92,
  E93,
  E94,
  E95,
  E96,
  E97,
  E98,
  E99,
  E100,
  E101,
  E102,
  E103,
  E104,
  E105,
  E106,
  E107,
  E108,
  E109,
  E110,
  E111,
  E112,
  E113,
  E114,
  E115,
  E116,
  E117,
  E118,
  E119,
  E120,
  E121,
  E122,
  E123,
  E124,
  E125,
  E126,
  E127,
  E128,
  E129,
  E130,
  E131,
  E132,
  E133,
  E134,
  E135,
  E136,
  E137,
  E138,
  E139,
  E140,
  E141,
  E142,
  E143,
  E144,
  E145,
  E146,
  E147,
  E148,
  E149,
  E150,
  E151,
  E152,
  E153,
  E154,
  E155,
  E156,
  E157,
  E158,
  E159,
  E160,
  E161,
  E162,
  E163,
  E164,
  E165,
  E166,
  E167,
  E168,
  E169,
  E170,
  E171,
  E172,
  E173,
  E174,
  E175,
  E176,
  E177,
  E178,
  E179,
  E180,
  E181,
  E182,
  E183,
  E184,
  E185,
  E186,
  E187,
  E188,
  E189,
  E190,
  E191,
  E192,
  E193,
  E194,
  E195,
  E196,
  E197,
  E198,
  E199,
  E200,
  E201,
  E202,
  E203,
  E204,
  E205,
  E206,
  E207,
  E208,
  E209,
  E210,
  E211,
  E212,
  E213,
  E214,
  E215,
  E216,
  E217,
  E218,
  E219,
  E220,
  E221,
  E222,
  E223,
  E224,
  E225,
  E226,
  E227,
  E228,
  E229,
  E230,
  E231,
  E232,
  E233,
  E234,
  E235,
  E236,
  E237,
  E238,
  E239,
  E240,
  E241,
  E242,
  E243,
  E244,
  E245,
  E246,
  E247,
  E248,
  E249,
  E250,
  E251,
  E252,
  E253,
  E254,
  E255,
  E256,
  E257,
  E258,
  E259,
  E260,
  E261,
  E262,
  E263,
  E264,
  E265,
  E266,
  E267,
  E268,
  E269,
  E270,
  E271,
  E272,
  E273,
  E274,
  E275,
  E276,
  E277,
  E278,
  E279,
  E280,
  E281,
  E282,
  E283,
  E284,
  E285,
  E286,
  E287,
  E288,
  E289,
  E290,
  E291,
  E292,
  E293,
  E294,
  E295,
  E296,
  E297,
  E298,
  E299,
  E300,
  E301,
  E302,
  E303,
  E304,
  E305,
  E306,
  E307,
  E308,
  E309,
  E310,
  E311,
  E312,
  E313,
  E314,
  E315,
  E316,
  E317,
  E318,
  E319,
  E320,
  E321,
  E322,
  E323,
  E324,
  E325,
  E326,
  E327,
  E328,
  E329,
  E330,
  E331,
  E332,
  E333,
  E334,
  E335,
  E336,
  E337,
  E338,
  E339,
  E340,
  E341,
  E342,
  E343,
  E344,
  E345,
  E346,
  E347,
  E348,
  E349,
  E350,
  E351,
  E352,
  E353,
  E354,
  E355,
  E356,
  E357,
  E358,
  E359,
  E360,
  E361,
  E362,
  E363,
  E364,
  E365,
  E366,
  E367,
  E368,
  E369,
  E370,
  E371,
  E372,
  E373,
  E374,
  E375,
  E376,
  E377,
  E378,
  E379,
  E380,
  E381,
  E382,
  E383,
  E384,
  E385,
  E386,
  E387,
  E388,
  E389,
  E390,
  E391,
  E392,
  E393,
  E394,
  E395,
  E396,
  E397,
  E398,
  E399,
  E400,
  E401,
  E402,
  E403,
  E404,
  E405,
  E406,
  E407,
  E408,
  E409,
  E410,
  E411,
  E412,
  E413,
  E414,
  E415,
  E416,
  E417,
  E418,
  E419,
  E420,
  E421,
  E422,
  E423,
  E424,
  E425,
  E426,
  E427,
  E428,
  E429,
  E430,
  E431,
  E432,
  E433,
  E434,
  E435,
  E436,
  E437,
  E438,
  E439,
  E440,
  E441,
  E442,
  E443,
  E444,
  E445,
  E446,
  E447,
  E448,
  E449,
  E450,
  E451,
  E452,
  E453,
  E454,
  E455,
  E456,
  E457,
  E458,
  E459,
  E460,
  E461,
  E462,
  E463,
  E464,
  E465,
  E466,
  E467,
  E468,
  E469,
  E470,
  E471,
  E472,
  E473,
  E474,
  E475,
  E476,
  E477,
  E478,
  E479,
  E480,
  E481,
  E482,
  E483,
  E484,
  E485,
  E486,
  E487,
  E488,
  E489,
  E490,
  E491,
  E492,
  E493,
  E494,
  E495,
  E496,
  E497,
  E498,
  E499,
  E500,
  E501,
  E502,
  E503,
  E504,
  E505,
  E506,
  E507,
  E508,
  E509,
  E510,
  E511,
  E512,
  E513,
  E514,
  E515,
  E516,
  E517,
  E518,
  E519,
  E520,
  E521,
  E522,
  E523,
  E524,
  E525,
  E526,
  E527,
  E528,
  E529,
  E530,
  E531,
  E532,
  E533,
  E534,
  E535,
  E536,
  E537,
  E538,
  E539,
  E540,
  E541,
  E542,
  E543,
  E544,
  E545,
  E546,
  E547,
  E548,
  E549,
  E550,
  E551,
  E552,
  E553,
  E554,
  E555,
  E556,
  E557,
  E558,
  E559,
  E560,
  E561,
  E562,
  E563,
  E564,
  E565,
  E566,
  E567,
  E568,
  E569,
  E570,
  E571,
  E572,
  E573,
  E574,
  E575,
  E576,
  E577,
  E578,
  E579,
  E580,
  E581,
  E582,
  E583,
  E584,
  E585,
  E586,
  E587,
  E588,
  E589,
  E590,
  E591,
  E592,
  E593,
  E594,
  E595,
  E596,
  E597,
  E598,
  E599,
  E600,
  E601,
  E602,
  E603,
  E604,
  E605,
  E606,
  E607,
  E608,
  E609,
  E610,
  E611,
  E612,
  E613,
  E614,
  E615,
  E616,
  E617,
  E618,
  E619,
  E620,
  E621,
  E622,
  E623,
  E624,
  E625,
  E626,
  E627,
  E628,
  E629,
  E630,
  E631,
  E632,
  E633,
  E634,
  E635,
  E636,
  E637,
  E638,
  E639,
  E640,
  E641,
  E642,
  E643,
  E644,
  E645,
  E646,
  E647,
  E648,
  E649,
  E650,
  E651,
  E652,
  E653,
  E654,
  E655,
  E656,
  E657,
  E658,
  E659,
  E660,
  E661,
  E662,
  E663,
  E664,
  E665,
  E666,
  E667,
  E668,
  E669,
  E670,
  E671,
  E672,
  E673,
  E674,
  E675,
  E676,
  E677,
  E678,
  E679,
  E680,
  E681,
  E682,
  E683,
  E684,
  E685,
  E686,
  E687,
  E688,
  E689,
  E690,
  E691,
  E692,
  E693,
  E694,
  E695,
  E696,
  E697,
  E698,
  E699,
  E700,
  E701,
  E702,
  E703,
  E704,
  E705,
  E706,
  E707,
  E708,
  E709,
  E710,
  E711,
  E712,
  E713,
  E714,
  E715,
  E716,
  E717,
  E718,
  E719,
  E720,
  E721,
  E722,
  E723,
  E724,
  E725,
  E726,
  E727,
  E728,
  E729,
  E730,
  E731,
  E732,
  E733,
  E734,
  E735,
  E736,
  E737,
  E738,
  E739,
  E740,
  E741,
  E742,
  E743,
  E744,
  E745,
  E746,
  E747,
  E748,
  E749,
  E750,
  E751,
  E752,
  E753,
  E754,
  E755,
  E756,
  E757,
  E758,
  E759,
  E760,
  E761,
  E762,
  E763,
  E764,
  E765,
  E766,
  E767,
  E768,
  E769,
  E770,
  E771,
  E772,
  E773,
  E774,
  E775,
  E776,
  E777,
  E778,
  E779,
  E780,
  E781,
  E782,
  E783,
  E784,
  E785,
  E786,
  E787,
  E788,
  E789,
  E790,
  E791,
  E792,
  E793,
  E794,
  E795,
  E796,
  E797,
  E798,
  E799,
  E800,
  E801,
  E802,
  E803,
  E804,
  E805,
  E806,
  E807,
  E808,
  E809,
  E810,
  E811,
  E812,
  E813,
  E814,
  E815,
  E816,
  E817,
  E818,
  E819,
  E820,
  E821,
  E822,
  E823,
  E824,
  E825,
  E826,
  E827,
  E828,
  E829,
  E830,
  E831,
  E832,
  E833,
  E834,
  E835,
  E836,
  E837,
  E838,
  E839,
  E840,
  E841,
  E842,
  E843,
  E844,
  E845,
  E846,
  E847,
  E848,
  E849,
  E850,
  E851,
  E852,
  E853,
  E854,
  E855,
  E856,
  E857,
  E858,
  E859,
  E860,
  E861,
  E862,
  E863,
  E864,
  E865,
  E866,
  E867,
  E868,
  E869,
  E870,
  E871,
  E872,
  E873,
  E874,
  E875,
  E876,
  E877,
  E878,
  E879,
  E880,
  E881,
  E882,
  E883,
  E884,
  E885,
  E886,
  E887,
  E888,
  E889,
  E890,
  E891,
  E892,
  E893,
  E894,
  E895,
  E896,
  E897,
  E898,
  E899,
  E900,
  E901,
  E902,
  E903,
  E904,
  E905,
  E906,
  E907,
  E908,
  E909,
  E910,
  E911,
  E912,
  E913,
  E914,
  E915,
  E916,
  E917,
  E918,
  E919,
  E920,
  E921,
  E922,
  E923,
  E924,
  E925,
  E926,
  E927,
  E928,
  E929,
  E930,
  E931,
  E932,
  E933,
  E934,
  E935,
  E936,
  E937,
  E938,
  E939,
  E940,
  E941,
  E942,
  E943,
  E944,
  E945,
  E946,
  E947,
  E948,
  E949,
  E950,
  E951,
  E952,
  E953,
  E954,
  E955,
  E956,
  E957,
  E958,
  E959,
  E960,
  E961,
  E962,
  E963,
  E964,
  E965,
  E966,
  E967,
  E968,
  E969,
  E970,
  E971,
  E972,
  E973,
  E974,
  E975,
  E976,
  E977,
  E978,
  E979,
  E980,
  E981,
  E982,
  E983,
  E984,
  E985,
  E986,
  E987,
  E988,
  E989,
  E990,
  E991,
  E992,
  E993,
  E994,
  E995,
  E996,
  E997,
  E998,
  E999,
  E1000,
  E1001,
  E1002,
  E1003,
  E1004,
  E1005,
  E1006,
  E1007,
  E1008,
  E1009,
  E1010,
  E1011,
  E1012,
  E1013,
  E1014,
  E1015,
  E1016,
  E1017,
  E1018,
  E1019,
  E1020,
  E1021,
  E1022,
  E1023,
}
function run(a: Int) {
    switch (a) {
        case 0:
            return [ E.E0, E.E1]
        case 2:
            return [ E.E0, E.E1]
        case 4:
            return [ E.E0, E.E1]
        case 6:
            return [ E.E0, E.E1]
        case 8:
            return [ E.E0, E.E1]
        case 10:
            return [ E.E0, E.E1]
        case 12:
            return [ E.E0, E.E1]
        case 14:
            return [ E.E0, E.E1]
        case 16:
            return [ E.E0, E.E1]
        case 18:
            return [ E.E0, E.E1]
        case 20:
            return [ E.E0, E.E1]
        case 22:
            return [ E.E0, E.E1]
        case 24:
            return [ E.E0, E.E1]
        case 26:
            return [ E.E0, E.E1]
        case 28:
            return [ E.E0, E.E1]
        case 30:
            return [ E.E0, E.E1]
        case 32:
            return [ E.E0, E.E1]
        case 34:
            return [ E.E0, E.E1]
        case 36:
            return [ E.E0, E.E1]
        case 38:
            return [ E.E0, E.E1]
        case 40:
            return [ E.E0, E.E1]
        case 42:
            return [ E.E0, E.E1]
        case 44:
            return [ E.E0, E.E1]
        case 46:
            return [ E.E0, E.E1]
        case 48:
            return [ E.E0, E.E1]
        case 50:
            return [ E.E0, E.E1]
        case 52:
            return [ E.E0, E.E1]
        case 54:
            return [ E.E0, E.E1]
        case 56:
            return [ E.E0, E.E1]
        case 58:
            return [ E.E0, E.E1]
        case 60:
            return [ E.E0, E.E1]
        case 62:
            return [ E.E0, E.E1]
        case 64:
            return [ E.E0, E.E1]
        case 66:
            return [ E.E0, E.E1]
        case 68:
            return [ E.E0, E.E1]
        case 70:
            return [ E.E0, E.E1]
        case 72:
            return [ E.E0, E.E1]
        case 74:
            return [ E.E0, E.E1]
        case 76:
            return [ E.E0, E.E1]
        case 78:
            return [ E.E0, E.E1]
        case 80:
            return [ E.E0, E.E1]
        case 82:
            return [ E.E0, E.E1]
        case 84:
            return [ E.E0, E.E1]
        case 86:
            return [ E.E0, E.E1]
        case 88:
            return [ E.E0, E.E1]
        case 90:
            return [ E.E0, E.E1]
        case 92:
            return [ E.E0, E.E1]
        case 94:
            return [ E.E0, E.E1]
        case 96:
            return [ E.E0, E.E1]
        case 98:
            return [ E.E0, E.E1]
        case 100:
            return [ E.E0, E.E1]
        case 102:
            return [ E.E0, E.E1]
        case 104:
            return [ E.E0, E.E1]
        case 106:
            return [ E.E0, E.E1]
        case 108:
            return [ E.E0, E.E1]
        case 110:
            return [ E.E0, E.E1]
        case 112:
            return [ E.E0, E.E1]
        case 114:
            return [ E.E0, E.E1]
        case 116:
            return [ E.E0, E.E1]
        case 118:
            return [ E.E0, E.E1]
        case 120:
            return [ E.E0, E.E1]
        case 122:
            return [ E.E0, E.E1]
        case 124:
            return [ E.E0, E.E1]
        case 126:
            return [ E.E0, E.E1]
        case 128:
            return [ E.E0, E.E1]
        case 130:
            return [ E.E0, E.E1]
        case 132:
            return [ E.E0, E.E1]
        case 134:
            return [ E.E0, E.E1]
        case 136:
            return [ E.E0, E.E1]
        case 138:
            return [ E.E0, E.E1]
        case 140:
            return [ E.E0, E.E1]
        case 142:
            return [ E.E0, E.E1]
        case 144:
            return [ E.E0, E.E1]
        case 146:
            return [ E.E0, E.E1]
        case 148:
            return [ E.E0, E.E1]
        case 150:
            return [ E.E0, E.E1]
        case 152:
            return [ E.E0, E.E1]
        case 154:
            return [ E.E0, E.E1]
        case 156:
            return [ E.E0, E.E1]
        case 158:
            return [ E.E0, E.E1]
        case 160:
            return [ E.E0, E.E1]
        case 162:
            return [ E.E0, E.E1]
        case 164:
            return [ E.E0, E.E1]
        case 166:
            return [ E.E0, E.E1]
        case 168:
            return [ E.E0, E.E1]
        case 170:
            return [ E.E0, E.E1]
        case 172:
            return [ E.E0, E.E1]
        case 174:
            return [ E.E0, E.E1]
        case 176:
            return [ E.E0, E.E1]
        case 178:
            return [ E.E0, E.E1]
        case 180:
            return [ E.E0, E.E1]
        case 182:
            return [ E.E0, E.E1]
        case 184:
            return [ E.E0, E.E1]
        case 186:
            return [ E.E0, E.E1]
        case 188:
            return [ E.E0, E.E1]
        case 190:
            return [ E.E0, E.E1]
        case 192:
            return [ E.E0, E.E1]
        case 194:
            return [ E.E0, E.E1]
        case 196:
            return [ E.E0, E.E1]
        case 198:
            return [ E.E0, E.E1]
        case 200:
            return [ E.E0, E.E1]
        case 202:
            return [ E.E0, E.E1]
        case 204:
            return [ E.E0, E.E1]
        case 206:
            return [ E.E0, E.E1]
        case 208:
            return [ E.E0, E.E1]
        case 210:
            return [ E.E0, E.E1]
        case 212:
            return [ E.E0, E.E1]
        case 214:
            return [ E.E0, E.E1]
        case 216:
            return [ E.E0, E.E1]
        case 218:
            return [ E.E0, E.E1]
        case 220:
            return [ E.E0, E.E1]
        case 222:
            return [ E.E0, E.E1]
        case 224:
            return [ E.E0, E.E1]
        case 226:
            return [ E.E0, E.E1]
        case 228:
            return [ E.E0, E.E1]
        case 230:
            return [ E.E0, E.E1]
        case 232:
            return [ E.E0, E.E1]
        case 234:
            return [ E.E0, E.E1]
        case 236:
            return [ E.E0, E.E1]
        case 238:
            return [ E.E0, E.E1]
        case 240:
            return [ E.E0, E.E1]
        case 242:
            return [ E.E0, E.E1]
        case 244:
            return [ E.E0, E.E1]
        case 246:
            return [ E.E0, E.E1]
        case 248:
            return [ E.E0, E.E1]
        case 250:
            return [ E.E0, E.E1]
        case 252:
            return [ E.E0, E.E1]
        case 254:
            return [ E.E0, E.E1]
        case 256:
            return [ E.E0, E.E1]
        case 258:
            return [ E.E0, E.E1]
        case 260:
            return [ E.E0, E.E1]
        case 262:
            return [ E.E0, E.E1]
        case 264:
            return [ E.E0, E.E1]
        case 266:
            return [ E.E0, E.E1]
        case 268:
            return [ E.E0, E.E1]
        case 270:
            return [ E.E0, E.E1]
        case 272:
            return [ E.E0, E.E1]
        case 274:
            return [ E.E0, E.E1]
        case 276:
            return [ E.E0, E.E1]
        case 278:
            return [ E.E0, E.E1]
        case 280:
            return [ E.E0, E.E1]
        case 282:
            return [ E.E0, E.E1]
        case 284:
            return [ E.E0, E.E1]
        case 286:
            return [ E.E0, E.E1]
        case 288:
            return [ E.E0, E.E1]
        case 290:
            return [ E.E0, E.E1]
        case 292:
            return [ E.E0, E.E1]
        case 294:
            return [ E.E0, E.E1]
        case 296:
            return [ E.E0, E.E1]
        case 298:
            return [ E.E0, E.E1]
        case 300:
            return [ E.E0, E.E1]
        case 302:
            return [ E.E0, E.E1]
        case 304:
            return [ E.E0, E.E1]
        case 306:
            return [ E.E0, E.E1]
        case 308:
            return [ E.E0, E.E1]
        case 310:
            return [ E.E0, E.E1]
        case 312:
            return [ E.E0, E.E1]
        case 314:
            return [ E.E0, E.E1]
        case 316:
            return [ E.E0, E.E1]
        case 318:
            return [ E.E0, E.E1]
        case 320:
            return [ E.E0, E.E1]
        case 322:
            return [ E.E0, E.E1]
        case 324:
            return [ E.E0, E.E1]
        case 326:
            return [ E.E0, E.E1]
        case 328:
            return [ E.E0, E.E1]
        case 330:
            return [ E.E0, E.E1]
        case 332:
            return [ E.E0, E.E1]
        case 334:
            return [ E.E0, E.E1]
        case 336:
            return [ E.E0, E.E1]
        case 338:
            return [ E.E0, E.E1]
        case 340:
            return [ E.E0, E.E1]
        case 342:
            return [ E.E0, E.E1]
        case 344:
            return [ E.E0, E.E1]
        case 346:
            return [ E.E0, E.E1]
        case 348:
            return [ E.E0, E.E1]
        case 350:
            return [ E.E0, E.E1]
        case 352:
            return [ E.E0, E.E1]
        case 354:
            return [ E.E0, E.E1]
        case 356:
            return [ E.E0, E.E1]
        case 358:
            return [ E.E0, E.E1]
        case 360:
            return [ E.E0, E.E1]
        case 362:
            return [ E.E0, E.E1]
        case 364:
            return [ E.E0, E.E1]
        case 366:
            return [ E.E0, E.E1]
        case 368:
            return [ E.E0, E.E1]
        case 370:
            return [ E.E0, E.E1]
        case 372:
            return [ E.E0, E.E1]
        case 374:
            return [ E.E0, E.E1]
        case 376:
            return [ E.E0, E.E1]
        case 378:
            return [ E.E0, E.E1]
        case 380:
            return [ E.E0, E.E1]
        case 382:
            return [ E.E0, E.E1]
        case 384:
            return [ E.E0, E.E1]
        case 386:
            return [ E.E0, E.E1]
        case 388:
            return [ E.E0, E.E1]
        case 390:
            return [ E.E0, E.E1]
        case 392:
            return [ E.E0, E.E1]
        case 394:
            return [ E.E0, E.E1]
        case 396:
            return [ E.E0, E.E1]
        case 398:
            return [ E.E0, E.E1]
        case 400:
            return [ E.E0, E.E1]
        case 402:
            return [ E.E0, E.E1]
        case 404:
            return [ E.E0, E.E1]
        case 406:
            return [ E.E0, E.E1]
        case 408:
            return [ E.E0, E.E1]
        case 410:
            return [ E.E0, E.E1]
        case 412:
            return [ E.E0, E.E1]
        case 414:
            return [ E.E0, E.E1]
        case 416:
            return [ E.E0, E.E1]
        case 418:
            return [ E.E0, E.E1]
        case 420:
            return [ E.E0, E.E1]
        case 422:
            return [ E.E0, E.E1]
        case 424:
            return [ E.E0, E.E1]
        case 426:
            return [ E.E0, E.E1]
        case 428:
            return [ E.E0, E.E1]
        case 430:
            return [ E.E0, E.E1]
        case 432:
            return [ E.E0, E.E1]
        case 434:
            return [ E.E0, E.E1]
        case 436:
            return [ E.E0, E.E1]
        case 438:
            return [ E.E0, E.E1]
        case 440:
            return [ E.E0, E.E1]
        case 442:
            return [ E.E0, E.E1]
        case 444:
            return [ E.E0, E.E1]
        case 446:
            return [ E.E0, E.E1]
        case 448:
            return [ E.E0, E.E1]
        case 450:
            return [ E.E0, E.E1]
        case 452:
            return [ E.E0, E.E1]
        case 454:
            return [ E.E0, E.E1]
        case 456:
            return [ E.E0, E.E1]
        case 458:
            return [ E.E0, E.E1]
        case 460:
            return [ E.E0, E.E1]
        case 462:
            return [ E.E0, E.E1]
        case 464:
            return [ E.E0, E.E1]
        case 466:
            return [ E.E0, E.E1]
        case 468:
            return [ E.E0, E.E1]
        case 470:
            return [ E.E0, E.E1]
        case 472:
            return [ E.E0, E.E1]
        case 474:
            return [ E.E0, E.E1]
        case 476:
            return [ E.E0, E.E1]
        case 478:
            return [ E.E0, E.E1]
        case 480:
            return [ E.E0, E.E1]
        case 482:
            return [ E.E0, E.E1]
        case 484:
            return [ E.E0, E.E1]
        case 486:
            return [ E.E0, E.E1]
        case 488:
            return [ E.E0, E.E1]
        case 490:
            return [ E.E0, E.E1]
        case 492:
            return [ E.E0, E.E1]
        case 494:
            return [ E.E0, E.E1]
        case 496:
            return [ E.E0, E.E1]
        case 498:
            return [ E.E0, E.E1]
        case 500:
            return [ E.E0, E.E1]
        case 502:
            return [ E.E0, E.E1]
        case 504:
            return [ E.E0, E.E1]
        case 506:
            return [ E.E0, E.E1]
        case 508:
            return [ E.E0, E.E1]
        case 510:
            return [ E.E0, E.E1]
        case 512:
            return [ E.E0, E.E1]
        case 514:
            return [ E.E0, E.E1]
        case 516:
            return [ E.E0, E.E1]
        case 518:
            return [ E.E0, E.E1]
        case 520:
            return [ E.E0, E.E1]
        case 522:
            return [ E.E0, E.E1]
        case 524:
            return [ E.E0, E.E1]
        case 526:
            return [ E.E0, E.E1]
        case 528:
            return [ E.E0, E.E1]
        case 530:
            return [ E.E0, E.E1]
        case 532:
            return [ E.E0, E.E1]
        case 534:
            return [ E.E0, E.E1]
        case 536:
            return [ E.E0, E.E1]
        case 538:
            return [ E.E0, E.E1]
        case 540:
            return [ E.E0, E.E1]
        case 542:
            return [ E.E0, E.E1]
        case 544:
            return [ E.E0, E.E1]
        case 546:
            return [ E.E0, E.E1]
        case 548:
            return [ E.E0, E.E1]
        case 550:
            return [ E.E0, E.E1]
        case 552:
            return [ E.E0, E.E1]
        case 554:
            return [ E.E0, E.E1]
        case 556:
            return [ E.E0, E.E1]
        case 558:
            return [ E.E0, E.E1]
        case 560:
            return [ E.E0, E.E1]
        case 562:
            return [ E.E0, E.E1]
        case 564:
            return [ E.E0, E.E1]
        case 566:
            return [ E.E0, E.E1]
        case 568:
            return [ E.E0, E.E1]
        case 570:
            return [ E.E0, E.E1]
        case 572:
            return [ E.E0, E.E1]
        case 574:
            return [ E.E0, E.E1]
        case 576:
            return [ E.E0, E.E1]
        case 578:
            return [ E.E0, E.E1]
        case 580:
            return [ E.E0, E.E1]
        case 582:
            return [ E.E0, E.E1]
        case 584:
            return [ E.E0, E.E1]
        case 586:
            return [ E.E0, E.E1]
        case 588:
            return [ E.E0, E.E1]
        case 590:
            return [ E.E0, E.E1]
        case 592:
            return [ E.E0, E.E1]
        case 594:
            return [ E.E0, E.E1]
        case 596:
            return [ E.E0, E.E1]
        case 598:
            return [ E.E0, E.E1]
        case 600:
            return [ E.E0, E.E1]
        case 602:
            return [ E.E0, E.E1]
        case 604:
            return [ E.E0, E.E1]
        case 606:
            return [ E.E0, E.E1]
        case 608:
            return [ E.E0, E.E1]
        case 610:
            return [ E.E0, E.E1]
        case 612:
            return [ E.E0, E.E1]
        case 614:
            return [ E.E0, E.E1]
        case 616:
            return [ E.E0, E.E1]
        case 618:
            return [ E.E0, E.E1]
        case 620:
            return [ E.E0, E.E1]
        case 622:
            return [ E.E0, E.E1]
        case 624:
            return [ E.E0, E.E1]
        case 626:
            return [ E.E0, E.E1]
        case 628:
            return [ E.E0, E.E1]
        case 630:
            return [ E.E0, E.E1]
        case 632:
            return [ E.E0, E.E1]
        case 634:
            return [ E.E0, E.E1]
        case 636:
            return [ E.E0, E.E1]
        case 638:
            return [ E.E0, E.E1]
        case 640:
            return [ E.E0, E.E1]
        case 642:
            return [ E.E0, E.E1]
        case 644:
            return [ E.E0, E.E1]
        case 646:
            return [ E.E0, E.E1]
        case 648:
            return [ E.E0, E.E1]
        case 650:
            return [ E.E0, E.E1]
        case 652:
            return [ E.E0, E.E1]
        case 654:
            return [ E.E0, E.E1]
        case 656:
            return [ E.E0, E.E1]
        case 658:
            return [ E.E0, E.E1]
        case 660:
            return [ E.E0, E.E1]
        case 662:
            return [ E.E0, E.E1]
        case 664:
            return [ E.E0, E.E1]
        case 666:
            return [ E.E0, E.E1]
        case 668:
            return [ E.E0, E.E1]
        case 670:
            return [ E.E0, E.E1]
        case 672:
            return [ E.E0, E.E1]
        case 674:
            return [ E.E0, E.E1]
        case 676:
            return [ E.E0, E.E1]
        case 678:
            return [ E.E0, E.E1]
        case 680:
            return [ E.E0, E.E1]
        case 682:
            return [ E.E0, E.E1]
        case 684:
            return [ E.E0, E.E1]
        case 686:
            return [ E.E0, E.E1]
        case 688:
            return [ E.E0, E.E1]
        case 690:
            return [ E.E0, E.E1]
        case 692:
            return [ E.E0, E.E1]
        case 694:
            return [ E.E0, E.E1]
        case 696:
            return [ E.E0, E.E1]
        case 698:
            return [ E.E0, E.E1]
        case 700:
            return [ E.E0, E.E1]
        case 702:
            return [ E.E0, E.E1]
        case 704:
            return [ E.E0, E.E1]
        case 706:
            return [ E.E0, E.E1]
        case 708:
            return [ E.E0, E.E1]
        case 710:
            return [ E.E0, E.E1]
        case 712:
            return [ E.E0, E.E1]
        case 714:
            return [ E.E0, E.E1]
        case 716:
            return [ E.E0, E.E1]
        case 718:
            return [ E.E0, E.E1]
        case 720:
            return [ E.E0, E.E1]
        case 722:
            return [ E.E0, E.E1]
        case 724:
            return [ E.E0, E.E1]
        case 726:
            return [ E.E0, E.E1]
        case 728:
            return [ E.E0, E.E1]
        case 730:
            return [ E.E0, E.E1]
        case 732:
            return [ E.E0, E.E1]
        case 734:
            return [ E.E0, E.E1]
        case 736:
            return [ E.E0, E.E1]
        case 738:
            return [ E.E0, E.E1]
        case 740:
            return [ E.E0, E.E1]
        case 742:
            return [ E.E0, E.E1]
        case 744:
            return [ E.E0, E.E1]
        case 746:
            return [ E.E0, E.E1]
        case 748:
            return [ E.E0, E.E1]
        case 750:
            return [ E.E0, E.E1]
        case 752:
            return [ E.E0, E.E1]
        case 754:
            return [ E.E0, E.E1]
        case 756:
            return [ E.E0, E.E1]
        case 758:
            return [ E.E0, E.E1]
        case 760:
            return [ E.E0, E.E1]
        case 762:
            return [ E.E0, E.E1]
        case 764:
            return [ E.E0, E.E1]
        case 766:
            return [ E.E0, E.E1]
        case 768:
            return [ E.E0, E.E1]
        case 770:
            return [ E.E0, E.E1]
        case 772:
            return [ E.E0, E.E1]
        case 774:
            return [ E.E0, E.E1]
        case 776:
            return [ E.E0, E.E1]
        case 778:
            return [ E.E0, E.E1]
        case 780:
            return [ E.E0, E.E1]
        case 782:
            return [ E.E0, E.E1]
        case 784:
            return [ E.E0, E.E1]
        case 786:
            return [ E.E0, E.E1]
        case 788:
            return [ E.E0, E.E1]
        case 790:
            return [ E.E0, E.E1]
        case 792:
            return [ E.E0, E.E1]
        case 794:
            return [ E.E0, E.E1]
        case 796:
            return [ E.E0, E.E1]
        case 798:
            return [ E.E0, E.E1]
        case 800:
            return [ E.E0, E.E1]
        case 802:
            return [ E.E0, E.E1]
        case 804:
            return [ E.E0, E.E1]
        case 806:
            return [ E.E0, E.E1]
        case 808:
            return [ E.E0, E.E1]
        case 810:
            return [ E.E0, E.E1]
        case 812:
            return [ E.E0, E.E1]
        case 814:
            return [ E.E0, E.E1]
        case 816:
            return [ E.E0, E.E1]
        case 818:
            return [ E.E0, E.E1]
        case 820:
            return [ E.E0, E.E1]
        case 822:
            return [ E.E0, E.E1]
        case 824:
            return [ E.E0, E.E1]
        case 826:
            return [ E.E0, E.E1]
        case 828:
            return [ E.E0, E.E1]
        case 830:
            return [ E.E0, E.E1]
        case 832:
            return [ E.E0, E.E1]
        case 834:
            return [ E.E0, E.E1]
        case 836:
            return [ E.E0, E.E1]
        case 838:
            return [ E.E0, E.E1]
        case 840:
            return [ E.E0, E.E1]
        case 842:
            return [ E.E0, E.E1]
        case 844:
            return [ E.E0, E.E1]
        case 846:
            return [ E.E0, E.E1]
        case 848:
            return [ E.E0, E.E1]
        case 850:
            return [ E.E0, E.E1]
        case 852:
            return [ E.E0, E.E1]
        case 854:
            return [ E.E0, E.E1]
        case 856:
            return [ E.E0, E.E1]
        case 858:
            return [ E.E0, E.E1]
        case 860:
            return [ E.E0, E.E1]
        case 862:
            return [ E.E0, E.E1]
        case 864:
            return [ E.E0, E.E1]
        case 866:
            return [ E.E0, E.E1]
        case 868:
            return [ E.E0, E.E1]
        case 870:
            return [ E.E0, E.E1]
        case 872:
            return [ E.E0, E.E1]
        case 874:
            return [ E.E0, E.E1]
        case 876:
            return [ E.E0, E.E1]
        case 878:
            return [ E.E0, E.E1]
        case 880:
            return [ E.E0, E.E1]
        case 882:
            return [ E.E0, E.E1]
        case 884:
            return [ E.E0, E.E1]
        case 886:
            return [ E.E0, E.E1]
        case 888:
            return [ E.E0, E.E1]
        case 890:
            return [ E.E0, E.E1]
        case 892:
            return [ E.E0, E.E1]
        case 894:
            return [ E.E0, E.E1]
        case 896:
            return [ E.E0, E.E1]
        case 898:
            return [ E.E0, E.E1]
        case 900:
            return [ E.E0, E.E1]
        case 902:
            return [ E.E0, E.E1]
        case 904:
            return [ E.E0, E.E1]
        case 906:
            return [ E.E0, E.E1]
        case 908:
            return [ E.E0, E.E1]
        case 910:
            return [ E.E0, E.E1]
        case 912:
            return [ E.E0, E.E1]
        case 914:
            return [ E.E0, E.E1]
        case 916:
            return [ E.E0, E.E1]
        case 918:
            return [ E.E0, E.E1]
        case 920:
            return [ E.E0, E.E1]
        case 922:
            return [ E.E0, E.E1]
        case 924:
            return [ E.E0, E.E1]
        case 926:
            return [ E.E0, E.E1]
        case 928:
            return [ E.E0, E.E1]
        case 930:
            return [ E.E0, E.E1]
        case 932:
            return [ E.E0, E.E1]
        case 934:
            return [ E.E0, E.E1]
        case 936:
            return [ E.E0, E.E1]
        case 938:
            return [ E.E0, E.E1]
        case 940:
            return [ E.E0, E.E1]
        case 942:
            return [ E.E0, E.E1]
        case 944:
            return [ E.E0, E.E1]
        case 946:
            return [ E.E0, E.E1]
        case 948:
            return [ E.E0, E.E1]
        case 950:
            return [ E.E0, E.E1]
        case 952:
            return [ E.E0, E.E1]
        case 954:
            return [ E.E0, E.E1]
        case 956:
            return [ E.E0, E.E1]
        case 958:
            return [ E.E0, E.E1]
        case 960:
            return [ E.E0, E.E1]
        case 962:
            return [ E.E0, E.E1]
        case 964:
            return [ E.E0, E.E1]
        case 966:
            return [ E.E0, E.E1]
        case 968:
            return [ E.E0, E.E1]
        case 970:
            return [ E.E0, E.E1]
        case 972:
            return [ E.E0, E.E1]
        case 974:
            return [ E.E0, E.E1]
        case 976:
            return [ E.E0, E.E1]
        case 978:
            return [ E.E0, E.E1]
        case 980:
            return [ E.E0, E.E1]
        case 982:
            return [ E.E0, E.E1]
        case 984:
            return [ E.E0, E.E1]
        case 986:
            return [ E.E0, E.E1]
        case 988:
            return [ E.E0, E.E1]
        case 990:
            return [ E.E0, E.E1]
        case 992:
            return [ E.E0, E.E1]
        case 994:
            return [ E.E0, E.E1]
        case 996:
            return [ E.E0, E.E1]
        case 998:
            return [ E.E0, E.E1]
        case 1000:
            return [ E.E0, E.E1]
        case 1002:
            return [ E.E0, E.E1]
        case 1004:
            return [ E.E0, E.E1]
        case 1006:
            return [ E.E0, E.E1]
        case 1008:
            return [ E.E0, E.E1]
        case 1010:
            return [ E.E0, E.E1]
        case 1012:
            return [ E.E0, E.E1]
        case 1014:
            return [ E.E0, E.E1]
        case 1016:
            return [ E.E0, E.E1]
        case 1018:
            return [ E.E0, E.E1]
        case 1020:
            return [ E.E0, E.E1]
        case 1022:
            return [ E.E0, E.E1]
      default :
        return [ E.E0, E.E1]
    }
}

function main() {
    arktest.assertEQ(run(1022)[0], E.E0)
    arktest.assertEQ(run(1022)[1], E.E1)
}
