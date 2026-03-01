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

//! CHECKER         If Conversion with SelectTransform JIT
//! SKIP_IF         @architecture != "arm64"
//! RUN             force_jit: true, options: "--compiler-loop-unroll=false", entry: "if_conversion_2.ETSGLOBAL::main"
//! EVENT           /Compilation,if_conversion_2.ETSGLOBAL::main,.*,COMPILED/
//! EVENT           /Compilation,if_conversion_2.ETSGLOBAL::__noinline__foo1,.*,COMPILED/
//! EVENT           /Compilation,if_conversion_2.ETSGLOBAL::__noinline__foo2,.*,COMPILED/
//! EVENT           /Compilation,if_conversion_2.ETSGLOBAL::__noinline__foo3,.*,COMPILED/
//! EVENT           /Compilation,if_conversion_2.ETSGLOBAL::__noinline__foo4,.*,COMPILED/
//! EVENT           /Compilation,if_conversion_2.ETSGLOBAL::__noinline__foo5,.*,COMPILED/
//! EVENT           /Compilation,if_conversion_2.ETSGLOBAL::__noinline__foo6,.*,COMPILED/
//! METHOD          "if_conversion_2.ETSGLOBAL::__noinline__foo1"
//! PASS_AFTER      "IfConversion"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      /SelectImmTransform GT i64 INC/, 1
//! INST_COUNT      /SelectImmTransform LE i64 INC/, 1
//! METHOD          "if_conversion_2.ETSGLOBAL::__noinline__foo2"
//! PASS_AFTER      "IfConversion"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      /SelectTransform (GT|GE) i32 NEG/, 1
//! INST_COUNT      /SelectTransform (LT|LE) i32 NEG/, 1
//! METHOD          "if_conversion_2.ETSGLOBAL::__noinline__foo3"
//! PASS_AFTER      "IfConversion"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      /SelectImmTransform GT i32 INV/, 1
//! INST_COUNT      /SelectImmTransform LE i32 INV/, 1
//! METHOD          "if_conversion_2.ETSGLOBAL::__noinline__foo4"
//! PASS_AFTER      "IfConversion"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      /SelectTransform (GT|GE) i64 INC/, 1
//! INST_COUNT      /SelectTransform (LT|LE) i64 INC/, 1
//! INST_COUNT      "SubI", 1
//! METHOD          "if_conversion_2.ETSGLOBAL::__noinline__foo5"
//! PASS_AFTER      "IfConversion"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      /SelectImmTransform GT i64 INC/, 1
//! INST_COUNT      /SelectImmTransform LE i64 INC/, 1
//! INST_COUNT      "SubI", 1
//! METHOD          "if_conversion_2.ETSGLOBAL::__noinline__foo6"
//! PASS_AFTER      "IfConversion"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      "SelectImm", 3
//! INST_COUNT      "Neg", 1
//! INST_COUNT      "AndI", 1

function __noinline__intToString(...z: int[]): string {
    return `${z}`;
}

function __noinline__longToString(...z: long[]): string {
    return `${z}`;
}

function __noinline__foo1(x: long, y: long): string {
    const z1 = (x + y > 42) ? x : y + 1;
    const z2 = (x - y > 42) ? x + 1 : y;
    return __noinline__longToString(z1, z2);
}

function __noinline__foo2(x: int, y: int, a: int): string {
    const z1 = (x + y > a) ? x : -y;
    const z2 = (x - y > a) ? -x : y;
    return __noinline__intToString(z1, z2);
}

function __noinline__foo3(x: int, y: int): string {
    const z1 = (x + y > 42) ? x : ~y;
    const z2 = (x - y > 42) ? ~x : y;
    return __noinline__intToString(z1, z2);
}

function __noinline__foo4(x: long, y: long, a: long): string {
    const z1 = (x + y > a) ? x + 10 : x + 9;
    const z2 = (x - y > a) ? y - 10 : y - 9;
    return __noinline__longToString(z1, z2);
}

function __noinline__foo5(x: long, y: long): string {
    const z1 = (x + y > 42) ? x + 10 : x + 11;
    const z2 = (x - y > 42) ? y - 10 : y - 11;
    return __noinline__longToString(z1, z2);
}

function __noinline__foo6(x: int, y: int): int {
    if (y > 42) {
        x += 10;
    } else {
        y += 10;
    }
    const z1 = -x & 127;
    const z2 = -y & 127;
    return (x < 84) ? z1 : z2;
}

function main(): void {
    arktest.assertEQ(__noinline__foo1(24, 8), "9,8");
    arktest.assertEQ(__noinline__foo1(40, 8), "40,8");
    arktest.assertEQ(__noinline__foo1(24, -36), "-35,25");
    arktest.assertEQ(__noinline__foo1(96, -36), "96,97");

    arktest.assertEQ(__noinline__foo2(24, 8, 42), "-8,8");
    arktest.assertEQ(__noinline__foo2(40, 8, 42), "40,8");
    arktest.assertEQ(__noinline__foo2(24, -36, 42), "36,-24");
    arktest.assertEQ(__noinline__foo2(96, -36, 42), "96,-96");

    arktest.assertEQ(__noinline__foo3(24, 8), "-9,8");
    arktest.assertEQ(__noinline__foo3(40, 8), "40,8");
    arktest.assertEQ(__noinline__foo3(24, -36), "35,-25");
    arktest.assertEQ(__noinline__foo3(96, -36), "96,-97");

    arktest.assertEQ(__noinline__foo4(24, 8, 42), "33,-1");
    arktest.assertEQ(__noinline__foo4(40, 8, 42), "50,-1");
    arktest.assertEQ(__noinline__foo4(24, -36, 42), "33,-46");
    arktest.assertEQ(__noinline__foo4(96, -36, 42), "106,-46");

    arktest.assertEQ(__noinline__foo5(24, 8), "35,-3");
    arktest.assertEQ(__noinline__foo5(40, 8), "50,-3");
    arktest.assertEQ(__noinline__foo5(24, -36), "35,-46");
    arktest.assertEQ(__noinline__foo5(96, -36), "106,-46");

    arktest.assertEQ(__noinline__foo6(70, 30), -70 & 127);
    arktest.assertEQ(__noinline__foo6(70, 50), -80 & 127);
    arktest.assertEQ(__noinline__foo6(90, 30), -40 & 127);
    arktest.assertEQ(__noinline__foo6(90, 50), -50 & 127);
}
