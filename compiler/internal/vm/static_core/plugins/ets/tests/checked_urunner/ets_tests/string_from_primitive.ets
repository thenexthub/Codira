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

//! CHECKER      Test creation string from primitive
//! RUN          force_jit: true, options: "--compiler-regex=.*noinline.*test.*", entry: "string_from_primitive.ETSGLOBAL::main"
//! METHOD       "string_from_primitive.ETSGLOBAL::__noinline__testStringFromBoolean"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT     "NewObject"
//! INST_NOT     /CallStatic.*std.*core.*Boolean.*ctor/
//! INST_COUNT   /CallStatic.*std.*core.*Boolean::toString/, 2
//! INST_COUNT   /CallStatic.*std.*core.*String::.*invoke/, 2
//!
//! METHOD       "string_from_primitive.ETSGLOBAL::__noinline__testStringFromByte"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT     "NewObject"
//! INST_NOT     /CallStatic.*std.*core.*Byte.*ctor/
//! INST_COUNT   /CallStatic.*std.*core.*Byte::toString/, 4
//! INST_COUNT   /CallStatic.*std.*core.*String::.*invoke/, 4
//!
//! METHOD       "string_from_primitive.ETSGLOBAL::__noinline__testStringFromChar"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT     "NewObject"
//! INST_NOT     /CallStatic.*std.*core.*Char.*ctor/
//! INST_COUNT   /CallStatic.*std.core.Char::toString/, 4
//! INST_COUNT   /CallStatic.*std.*core.*String::.*invoke/, 4
//!
//! METHOD       "string_from_primitive.ETSGLOBAL::__noinline__testStringFromShort"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT     "NewObject"
//! INST_NOT     /CallStatic.*std.*core.*Short.*ctor/
//! INST_COUNT   /CallStatic.*std.*core.*Short::toString/, 4
//! INST_COUNT   /CallStatic.*std.*core.*String::.*invoke/, 4
//!
//! METHOD       "string_from_primitive.ETSGLOBAL::__noinline__testStringFromInt"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT     "NewObject"
//! INST_NOT     /CallStatic.*std.*core.*Int.*ctor/
//! INST_COUNT   /CallStatic.*std.*core.*Int::toString/, 4
//! INST_COUNT   /CallStatic.*std.*core.*String::.*invoke/, 4
//!
//! METHOD       "string_from_primitive.ETSGLOBAL::__noinline__testStringFromLong"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT     "NewObject"
//! INST_NOT     /CallStatic.*std.*core.*Long.*ctor/
//! INST_COUNT   /CallStatic.*std.*core.*Long::toString/, 4
//! INST_COUNT   /CallStatic.*std.*core.*String::.*invoke/, 4
//!
//! METHOD       "string_from_primitive.ETSGLOBAL::__noinline__testStringFromFloat"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT     "NewObject"
//! INST_NOT     /CallStatic.*std.*core.*Float.*ctor/
//! INST_COUNT   /CallStatic.*std.*core.*Float::toString/, 7
//! INST_COUNT   /CallStatic.*std.*core.*String::.*invoke/, 7
//!
//! METHOD       "string_from_primitive.ETSGLOBAL::__noinline__testStringFromDouble"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT     "NewObject"
//! INST_NOT     /CallStatic.*std.*core.*Double.*ctor/
//! INST_COUNT   /CallStatic.*std.*core.*Double::toString/, 7
//! INST_COUNT   /CallStatic.*std.*core.*String::.*invoke/, 7

function __noinline__testStringFromBoolean(): void {
    let t: boolean = true;
    let f: boolean = false;
    let ts: String = new String(t);
    let fs: String = new String(f);
    arktest.assertEQ(ts, "true");
    arktest.assertEQ(fs, "false");
    arktest.assertEQ(String(t), "true");
    arktest.assertEQ(String(f), "false");
}

function __noinline__testStringFromByte(): void {
    let b1: byte = 1;
    let b2: byte = -1;
    let b3: byte = 127;
    let b4: byte = -128;
    let bs1: String = new String(b1);
    let bs2: String = new String(b2);
    let bs3: String = new String(b3);
    let bs4: String = new String(b4);
    arktest.assertEQ(bs1, "1");
    arktest.assertEQ(bs2, "-1");
    arktest.assertEQ(bs3, "127");
    arktest.assertEQ(bs4, "-128");
    arktest.assertEQ(String(b1), "1");
    arktest.assertEQ(String(b2), "-1");
    arktest.assertEQ(String(b3), "127");
    arktest.assertEQ(String(b4), "-128");
}

function __noinline__testStringFromChar(): void {
    let c1: char = c'a';
    let c2: char = c'z';
    let c3: char = c'A';
    let c4: char = c'Z';
    let cs1: String = new String(c1);
    let cs2: String = new String(c2);
    let cs3: String = new String(c3);
    let cs4: String = new String(c4);
    arktest.assertEQ(cs1, "a");
    arktest.assertEQ(cs2, "z");
    arktest.assertEQ(cs3, "A");
    arktest.assertEQ(cs4, "Z");
    arktest.assertEQ(String(c1), "a");
    arktest.assertEQ(String(c2), "z");
    arktest.assertEQ(String(c3), "A");
    arktest.assertEQ(String(c4), "Z");
}

function __noinline__testStringFromShort(): void {
    let s1: short = 1;
    let s2: short = -1;
    let s3: short = 32767;
    let s4: short = -32768;
    let ss1: String = new String(s1);
    let ss2: String = new String(s2);
    let ss3: String = new String(s3);
    let ss4: String = new String(s4);
    arktest.assertEQ(ss1, "1");
    arktest.assertEQ(ss2, "-1");
    arktest.assertEQ(ss3, "32767");
    arktest.assertEQ(ss4, "-32768");
    arktest.assertEQ(String(s1), "1");
    arktest.assertEQ(String(s2), "-1");
    arktest.assertEQ(String(s3), "32767");
    arktest.assertEQ(String(s4), "-32768");
}

function __noinline__testStringFromInt(): void {
    let i1: int = 1;
    let i2: int = -1;
    let i3: int = 2147483647;
    let i4: int = -2147483648;
    let is1: String = new String(i1);
    let is2: String = new String(i2);
    let is3: String = new String(i3);
    let is4: String = new String(i4);
    arktest.assertEQ(is1, "1");
    arktest.assertEQ(is2, "-1");
    arktest.assertEQ(is3, "2147483647");
    arktest.assertEQ(is4, "-2147483648");
    arktest.assertEQ(String(i1), "1");
    arktest.assertEQ(String(i2), "-1");
    arktest.assertEQ(String(i3), "2147483647");
    arktest.assertEQ(String(i4), "-2147483648");
}

function __noinline__testStringFromLong(): void {
    let l1: long = 1;
    let l2: long = -1;
    let l3: long = 9223372036854775807;
    let l4: long = -9223372036854775808;
    let ls1: String = new String(l1);
    let ls2: String = new String(l2);
    let ls3: String = new String(l3);
    let ls4: String = new String(l4);
    arktest.assertEQ(ls1, "1");
    arktest.assertEQ(ls2, "-1");
    arktest.assertEQ(ls3, "9223372036854775807");
    arktest.assertEQ(ls4, "-9223372036854775808");
    arktest.assertEQ(String(l1), "1");
    arktest.assertEQ(String(l2), "-1");
    arktest.assertEQ(String(l3), "9223372036854775807");
    arktest.assertEQ(String(l4), "-9223372036854775808");
}

function __noinline__testStringFromFloat(): void {
    let f1: float = 1.0f;
    let f2: float = -1.0f;
    let f3: float = 3.4028235e+38f;
    let f4: float = -3.4028235e+38f;
    let fs1: String = new String(f1);
    let fs2: String = new String(f2);
    let fs3: String = new String(f3);
    let fs4: String = new String(f4);
    let fs5: String = new String(Float.NaN);
    let fs6: String = new String(Float.POSITIVE_INFINITY)
    let fs7: String = new String(Float.NEGATIVE_INFINITY)
    arktest.assertEQ(fs1, "1");
    arktest.assertEQ(fs2, "-1");
    arktest.assertEQ(fs3, "3.4028235e+38");
    arktest.assertEQ(fs4, "-3.4028235e+38");
    arktest.assertEQ(fs5, "NaN");
    arktest.assertEQ(fs6, "Infinity");
    arktest.assertEQ(fs7, "-Infinity");
    arktest.assertEQ(String(f1), "1");
    arktest.assertEQ(String(f2), "-1");
    arktest.assertEQ(String(f3), "3.4028235e+38");
    arktest.assertEQ(String(f4), "-3.4028235e+38");
    arktest.assertEQ(String(Float.NaN), "NaN");
    arktest.assertEQ(String(Float.POSITIVE_INFINITY), "Infinity");
    arktest.assertEQ(String(Float.NEGATIVE_INFINITY), "-Infinity");
}

function __noinline__testStringFromDouble(): void {
    let d1: double = 1.0;
    let d2: double = -1.0;
    let d3: double = 1.7976931348623157e+308;
    let d4: double = -1.7976931348623157e+308;
    let ds1: String = new String(d1);
    let ds2: String = new String(d2);
    let ds3: String = new String(d3);
    let ds4: String = new String(d4);
    let ds5: String = new String(Double.NaN);
    let ds6: String = new String(Double.POSITIVE_INFINITY)
    let ds7: String = new String(Double.NEGATIVE_INFINITY)
    arktest.assertEQ(ds1, "1");
    arktest.assertEQ(ds2, "-1");
    arktest.assertEQ(ds3, "1.7976931348623157e+308");
    arktest.assertEQ(ds4, "-1.7976931348623157e+308");
    arktest.assertEQ(ds5, "NaN");
    arktest.assertEQ(ds6, "Infinity");
    arktest.assertEQ(ds7, "-Infinity");
    arktest.assertEQ(String(d1), "1");
    arktest.assertEQ(String(d2), "-1");
    arktest.assertEQ(String(d3), "1.7976931348623157e+308");
    arktest.assertEQ(String(d4), "-1.7976931348623157e+308");
    arktest.assertEQ(String(Double.NaN), "NaN");
    arktest.assertEQ(String(Double.POSITIVE_INFINITY), "Infinity");
    arktest.assertEQ(String(Double.NEGATIVE_INFINITY), "-Infinity");
}

function main(): int {
    __noinline__testStringFromBoolean();
    __noinline__testStringFromByte();
    __noinline__testStringFromChar();
    __noinline__testStringFromShort();
    __noinline__testStringFromInt();
    __noinline__testStringFromLong();
    __noinline__testStringFromFloat();
    __noinline__testStringFromDouble();
    return 0;
}
