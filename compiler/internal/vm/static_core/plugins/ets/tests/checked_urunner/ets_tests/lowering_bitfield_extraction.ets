/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

//! CHECKER         Peepholes with Bitfield Extration JIT for AArch64
//! SKIP_IF         @architecture != "arm64"
//! RUN             force_jit: true, options: "--compiler-loop-unroll=false", entry: "lowering_bitfield_extraction.ETSGLOBAL::main"
//! EVENT           /Compilation,lowering_bitfield_extraction.ETSGLOBAL::main,.*,COMPILED/
//! EVENT           /Compilation,lowering_bitfield_extraction.ETSGLOBAL::__noinline__foo1,.*,COMPILED/
//! EVENT           /Compilation,lowering_bitfield_extraction.ETSGLOBAL::__noinline__foo2,.*,COMPILED/
//! EVENT           /Compilation,lowering_bitfield_extraction.ETSGLOBAL::__noinline__foo3,.*,COMPILED/
//! EVENT           /Compilation,lowering_bitfield_extraction.ETSGLOBAL::__noinline__foo4,.*,COMPILED/
//! EVENT           /Compilation,lowering_bitfield_extraction.ETSGLOBAL::__noinline__foo5,.*,COMPILED/
//! EVENT           /Compilation,lowering_bitfield_extraction.ETSGLOBAL::__noinline__foo6,.*,COMPILED/
//! METHOD          "lowering_bitfield_extraction.ETSGLOBAL::__noinline__foo1"
//! PASS_AFTER      "Lowering"
//! INST_COUNT      "ExtractBitfield", 4
//! PASS_AFTER_NEXT "Cleanup"
//! INST_NOT        "Shr"
//! INST_NOT        "AShr"
//! INST_NOT        "And"
//! METHOD          "lowering_bitfield_extraction.ETSGLOBAL::__noinline__foo2"
//! PASS_AFTER      "Lowering"
//! INST_NOT        "ExtractBitfield"
//! METHOD          "lowering_bitfield_extraction.ETSGLOBAL::__noinline__foo3"
//! PASS_AFTER      "Lowering"
//! INST_COUNT      "ExtractBitfield", 4
//! PASS_AFTER_NEXT "Cleanup"
//! INST_NOT        "Shl"
//! INST_NOT        "Shr"
//! INST_NOT        "AShr"
//! METHOD          "lowering_bitfield_extraction.ETSGLOBAL::__noinline__foo4"
//! PASS_AFTER      "Lowering"
//! INST_NOT        "ExtractBitfield"
//! METHOD          "lowering_bitfield_extraction.ETSGLOBAL::__noinline__foo5"
//! PASS_AFTER      "Lowering"
//! INST_COUNT      "ExtractBitfield", 6
//! PASS_AFTER_NEXT "Cleanup"
//! INST_NOT        "And"
//! INST_NOT        "Shr"
//! INST_NOT        "AShr"
//! METHOD          "lowering_bitfield_extraction.ETSGLOBAL::__noinline__foo6"
//! PASS_AFTER      "Lowering"
//! INST_NOT        "ExtractBitfield"

//! CHECKER         Peepholes with Bitfield Extration JIT for Arm32 (bfx only supported for 32-bit integers)
//! SKIP_IF         @architecture != "arm32"
//! RUN             force_jit: true, options: "--compiler-regex='.*ETSGLOBAL::.*' --compiler-loop-unroll=false", entry: "lowering_bitfield_extraction.ETSGLOBAL::main"
//! EVENT           /Compilation,lowering_bitfield_extraction.ETSGLOBAL::main,.*,COMPILED/
//! EVENT           /Compilation,lowering_bitfield_extraction.ETSGLOBAL::__noinline__foo1,.*,COMPILED/
//! EVENT           /Compilation,lowering_bitfield_extraction.ETSGLOBAL::__noinline__foo2,.*,COMPILED/
//! EVENT           /Compilation,lowering_bitfield_extraction.ETSGLOBAL::__noinline__foo3,.*,COMPILED/
//! EVENT           /Compilation,lowering_bitfield_extraction.ETSGLOBAL::__noinline__foo4,.*,COMPILED/
//! EVENT           /Compilation,lowering_bitfield_extraction.ETSGLOBAL::__noinline__foo5,.*,COMPILED/
//! EVENT           /Compilation,lowering_bitfield_extraction.ETSGLOBAL::__noinline__foo6,.*,COMPILED/
//! METHOD          "lowering_bitfield_extraction.ETSGLOBAL::__noinline__foo1"
//! PASS_AFTER      "Lowering"
//! INST_COUNT      "ExtractBitfield", 2
//! METHOD          "lowering_bitfield_extraction.ETSGLOBAL::__noinline__foo2"
//! PASS_AFTER      "Lowering"
//! INST_NOT        "ExtractBitfield"
//! METHOD          "lowering_bitfield_extraction.ETSGLOBAL::__noinline__foo3"
//! PASS_AFTER      "Lowering"
//! INST_COUNT      "ExtractBitfield", 2
//! METHOD          "lowering_bitfield_extraction.ETSGLOBAL::__noinline__foo4"
//! PASS_AFTER      "Lowering"
//! INST_NOT        "ExtractBitfield"
//! METHOD          "lowering_bitfield_extraction.ETSGLOBAL::__noinline__foo5"
//! PASS_AFTER      "Lowering"
//! INST_COUNT      "ExtractBitfield", 3
//! METHOD          "lowering_bitfield_extraction.ETSGLOBAL::__noinline__foo6"
//! PASS_AFTER      "Lowering"
//! INST_NOT        "ExtractBitfield"

function __noinline__foo1(x: int, y: int, z: long, w: long): long[] {
    x = (x >> 27) & 0x1F;
    y = (y >>> 10) & 0x3F;
    z = (z >> 18) & 0x3F;
    w = (w >>> 28) & 0x3F;
    return [Int.toLong(x), Int.toLong(y), z, w];
}

function __noinline__foo2(x: int, y: int, z: long, w: long): long[] {
    x = (x >> 27) & 0x3F;
    y = (y >>> 25) & 0x3FF;
    z = (z >> 51) & 0x3FFF;
    w = (w >>> 36) & 0x3FFF_FFFF;
    return [Int.toLong(x), Int.toLong(y), z, w];
}

function __noinline__foo3(x: int, y: int, z: long, w: long): long[] {
    x = (x << 3) >> 3;
    y = (y << 9) >>> 24;
    z = (z << 13) >> 35;
    w = (w << 25) >>> 46;
    return [Int.toLong(x), Int.toLong(y), z, w];
}

function __noinline__foo4(x: int, y: int, z: long, w: long): long[] {
    x = (x << 3) >> 2;
    y = (y << 30) >>> 26;
    z = (z << 19) >> 12;
    w = (w << 45) >>> 44;
    return [Int.toLong(x), Int.toLong(y), z, w];
}

function __noinline__foo5(x: int, y: int, z: long, w: long): long[] {
    x = (x & 0x3FFF) >> 5;
    const y1: int = (y & -3804) >> 12;  // -3804 = 2's complement of 0xFFFF_F124
    const y2: int = (y & -4096) >>> 12; // -4096 = 2's complement of 0xFFFF_F000
    z = (z & 0x3FFF_FFFF_FF00) >>> 15;
    const w1: long = (w & (-0x10000)) >> 16;
    const w2: long = (w & (-0xFEDC)) >>> 16;
    return [Int.toLong(x), Int.toLong(y1), Int.toLong(y2), z, w1, w2];
}

function __noinline__foo6(x: int, y: int, z: long, w: long): long[] {
    x = (x & 0x3FFF) >> 14;
    y = (y & 0x1F_F800) >>> 10;
    z = (z & 0x3FFF_FFFF_EF00) >>> 11;
    w = (w & 0x8F_FFFF_FFFF_FFFF) >> 12;
    return [Int.toLong(x), Int.toLong(y), z, w];
}

function main(): void {
    const a: int   = -0b100_1000_1000_0100_0001_0000_0010_0001;
    const b: long  = -0b100_1000_1000_0100_0001_0000_0010_0000_0010_0000_0001_0000_0000_0100_0000_0001;

    const a1 = __noinline__foo1(a, a, b, b);
    arktest.assertEQ(a1[0], 0b1_0110);
    arktest.assertEQ(a1[1], 0b11_1011);
    arktest.assertEQ(a1[2], 0b11_1011);
    arktest.assertEQ(a1[3], 0b11_1101);

    const a2 = __noinline__foo2(a, a, b, b);
    arktest.assertEQ(a2[0], 0b11_0110);
    arktest.assertEQ(a2[1], 0b00_0101_1011);
    arktest.assertEQ(a2[2], 0b11_0110_1110_1111);
    arktest.assertEQ(a2[3], 0b00_1011_0111_0111_1011_1110_1111_1101);

    const a3 = __noinline__foo3(a, a, b, b);
    arktest.assertEQ(a3[0], -0b0000_1000_1000_0100_0001_0000_0010_0001);
    arktest.assertEQ(a3[1], 0b1111_0111);
    arktest.assertEQ(a3[2], 0b0_1111_1011_1111_0111_1111_0111_1111);
    arktest.assertEQ(a3[3], 0b10_1111_1110_1111_1111);

    const a4 = __noinline__foo4(a, a, b, b);
    arktest.assertEQ(a4[0], -0b0001_0001_0000_1000_0010_0000_0100_0010);
    arktest.assertEQ(a4[1], 0b11_0000);
    arktest.assertEQ(a4[2], 0b0111_1110_1111_1110_1111_1111_0111_1111_1101_1111_1111_1000_0000);
    arktest.assertEQ(a4[3], 0b1111_1111_0111_1111_1110);

    const a5 = __noinline__foo5(a, a, b, b);
    arktest.assertEQ(a5[0], 0b1_0111_1110);
    arktest.assertEQ(a5[1], -0b0000_0000_0000_0100_1000_1000_0100_0010);
    arktest.assertEQ(a5[2], 0b0000_0000_0000_1011_0111_0111_1011_1110);
    arktest.assertEQ(a5[3], 0b101_1111_1011_1111_1011_1111_1101_1111);
    arktest.assertEQ(a5[4], -0b0100_1000_1000_0100_0001_0000_0010_0000_0010_0000_0001_0001);
    arktest.assertEQ(a5[5], 0b0000_0000_0000_0000_1011_0111_0111_1011_1110_1111_1101_1111_1101_1111_1110_1111);

    const a6 = __noinline__foo6(a, a, b, b);
    arktest.assertEQ(a6[0], 0);
    arktest.assertEQ(a6[1], 0b110_1111_1010);
    arktest.assertEQ(a6[2], 0b101_1111_1011_1111_1011_1111_1101_1111_1101);
    arktest.assertEQ(a6[3], 0b1011_1110_1111_1101_1111_1101_1111_1110_1111_1111);
}
