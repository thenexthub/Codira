/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

export const bin = 0b101, binBig = 0b101n;
export const oct = 0o567, octBig = 0o567n;
export const hex = 0xC0B, hexBig = 0xC0Bn;
export const dec = 123,   decBig = 123n;

export const largeBin = 0b10101010101010101010101010101010101010101010101010101010101n;
export const largeOct = 0o123456712345671234567n;
export const largeDec = 12345678091234567890n;
export const largeHex = 0x1234567890abcdefn;
 
export const separatedBin = 0b010_10_1n;
export const separatedOct = 0o1234_567n;
export const separatedDec = 123_456_789n;
export const separatedHex = 0x0_abcdefn;
 
export const zero         = 0b0n;
export const oneBit       = 0b1n;
export const twoBit       = 0b11n;
export const threeBit     = 0b111n;
 
export const neg = -123n;
export const negHex: -16n = -0x10n;
export let threeBitLet = 0b111n;

export const negZero: 0n = -0n;
export const baseChange: 255n = 0xFFn;
export const leadingZeros: 0xFFn = 0x000000FFn;

export let bigint_v:123n=123n;
export const baseChange_v: 255n = 0xFFn;

export let zero_v = 0b0n;
export let oneBit_v = 0b1n;
export let twoBit_v = 0b11n;
export let threeBit_v = 0b111n;
