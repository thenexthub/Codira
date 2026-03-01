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

//! DEFINE_FRONTEND_OPTIONS            DEFAULT ARGS
//! RUN_FRONTEND            options: "--ets-strings-concat=false"

//! CHECKER           AOT: Checking IsTrue optimization
//! SKIP_IF           @architecture == "arm32"
//! RUN_PAOC          options: "--compiler-regex='.*test.*'", entry: "optimize_istrue.ETSGLOBAL::main"
//!
//! # testByte
//! METHOD            "optimize_istrue.ETSGLOBAL::testByteFalse"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Byte\.value/
//! INST_NOT          "Compare NE i8"
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Byte\.value/
//! INST_NEXT         "Compare NE i8"
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testByteTrue"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Byte\.value/
//! INST_NOT          "Compare NE i8"
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Byte\.value/
//! INST_NEXT         "Compare NE i8"
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! # testChar
//! METHOD            "optimize_istrue.ETSGLOBAL::testCharFalse"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Char\.value/
//! INST_NOT          "Compare NE u16"
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Char\.value/
//! INST_NEXT         "Compare NE u16"
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testCharTrue"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Char\.value/
//! INST_NOT          "Compare NE u16"
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Char\.value/
//! INST_NEXT         "Compare NE u16"
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! # testShort
//! METHOD            "optimize_istrue.ETSGLOBAL::testShortFalse"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Short\.value/
//! INST_NOT          "Compare NE i16"
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Short\.value/
//! INST_NEXT         "Compare NE i16"
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testShortTrue"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Short\.value/
//! INST_NOT          "Compare NE i16"
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Short\.value/
//! INST_NEXT         "Compare NE i16"
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! # testInt
//! METHOD            "optimize_istrue.ETSGLOBAL::testIntFalse"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Int\.value/
//! INST_NOT          "Compare NE i32"
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Int\.value/
//! INST_NEXT         "Compare NE i32"
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testIntTrue"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Int\.value/
//! INST_NOT          "Compare NE i32"
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Int\.value/
//! INST_NEXT         "Compare NE i32"
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! # testLong
//! METHOD            "optimize_istrue.ETSGLOBAL::testLongFalse"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Long\.value/
//! INST_NOT          "Compare NE i64"
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Long\.value/
//! INST_NEXT         "Compare NE i64"
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testLongTrue"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Long\.value/
//! INST_NOT          "Compare NE i64"
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Long\.value/
//! INST_NEXT         "Compare NE i64"
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! # testFloat
//! METHOD            "optimize_istrue.ETSGLOBAL::testFloatPositiveInfinity"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Float\.value/
//! INST_NOT          "Compare EQ f32"
//! INST_NOT          "Compare NE f32"
//! INST_NOT          "And "
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Float\.value/
//! INST_NEXT         "Compare EQ f32"
//! INST_NEXT         "Compare NE f32"
//! INST_NEXT         "And "
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testFloatNegativeInfinity"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Float\.value/
//! INST_NOT          "Compare EQ f32"
//! INST_NOT          "Compare NE f32"
//! INST_NOT          "And "
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Float\.value/
//! INST_NEXT         "Compare EQ f32"
//! INST_NEXT         "Compare NE f32"
//! INST_NEXT         "And "
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testFloatNaN"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Float\.value/
//! INST_NOT          "Compare EQ f32"
//! INST_NOT          "Compare NE f32"
//! INST_NOT          "And "
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Float\.value/
//! INST_NEXT         "Compare EQ f32"
//! INST_NEXT         "Compare NE f32"
//! INST_NEXT         "And "
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testSignedFloatNaN"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Float\.value/
//! INST_NOT          "Compare EQ f32"
//! INST_NOT          "Compare NE f32"
//! INST_NOT          "And "
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Float\.value/
//! INST_NEXT         "Compare EQ f32"
//! INST_NEXT         "Compare NE f32"
//! INST_NEXT         "And "
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testFloatFalse"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Float\.value/
//! INST_NOT          "Compare EQ f32"
//! INST_NOT          "Compare NE f32"
//! INST_NOT          "And "
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Float\.value/
//! INST_NEXT         "Compare EQ f32"
//! INST_NEXT         "Compare NE f32"
//! INST_NEXT         "And "
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testFloatTrue"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Float\.value/
//! INST_NOT          "Compare EQ f32"
//! INST_NOT          "Compare NE f32"
//! INST_NOT          "And "
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Float\.value/
//! INST_NEXT         "Compare EQ f32"
//! INST_NEXT         "Compare NE f32"
//! INST_NEXT         "And "
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! # testDouble
//! METHOD            "optimize_istrue.ETSGLOBAL::testDoublePositiveInfinity"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Double\.value/
//! INST_NOT          "Compare EQ f64"
//! INST_NOT          "Compare NE f64"
//! INST_NOT          "And "
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Double\.value/
//! INST_NEXT         "Compare EQ f64"
//! INST_NEXT         "Compare NE f64"
//! INST_NEXT         "And "
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testDoubleNegativeInfinity"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Double\.value/
//! INST_NOT          "Compare EQ f64"
//! INST_NOT          "Compare NE f64"
//! INST_NOT          "And "
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Double\.value/
//! INST_NEXT         "Compare EQ f64"
//! INST_NEXT         "Compare NE f64"
//! INST_NEXT         "And "
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testDoubleNaN"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Double\.value/
//! INST_NOT          "Compare EQ f64"
//! INST_NOT          "Compare NE f64"
//! INST_NOT          "And "
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Double\.value/
//! INST_NEXT         "Compare EQ f64"
//! INST_NEXT         "Compare NE f64"
//! INST_NEXT         "And "
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testSignedDoubleNaN"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Double\.value/
//! INST_NOT          "Compare EQ f64"
//! INST_NOT          "Compare NE f64"
//! INST_NOT          "And "
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Double\.value/
//! INST_NEXT         "Compare EQ f64"
//! INST_NEXT         "Compare NE f64"
//! INST_NEXT         "And "
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testDoubleFalse"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Double\.value/
//! INST_NOT          "Compare EQ f64"
//! INST_NOT          "Compare NE f64"
//! INST_NOT          "And "
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Double\.value/
//! INST_NEXT         "Compare EQ f64"
//! INST_NEXT         "Compare NE f64"
//! INST_NEXT         "And "
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testDoubleTrue"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Double\.value/
//! INST_NOT          "Compare EQ f64"
//! INST_NOT          "Compare NE f64"
//! INST_NOT          "And "
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Double\.value/
//! INST_NEXT         "Compare EQ f64"
//! INST_NEXT         "Compare NE f64"
//! INST_NEXT         "And "
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! # testString
//! METHOD            "optimize_istrue.ETSGLOBAL::testStringFalse"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          "LenArray"
//! INST_NOT          "Compare NE i32"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              "LenArray"
//! INST_NEXT         "Compare NE i32"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testStringTrue"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          "LenArray"
//! INST_NOT          "Compare NE i32"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              "LenArray"
//! INST_NEXT         "Compare NE i32"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! # testBoolean
//! METHOD            "optimize_istrue.ETSGLOBAL::testBooleanFalse"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Boolean\.value/
//! INST_NOT          "Compare NE b"
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Boolean\.value/
//! INST_NEXT         "Compare NE b"
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testBooleanTrue"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Boolean\.value/
//! INST_NOT          "Compare NE b"
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Boolean\.value/
//! INST_NEXT         "Compare NE b"
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! # testNull
//! METHOD            "optimize_istrue.ETSGLOBAL::__noinline__testNull"
//! PASS_AFTER        "Inline"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Inline"
//! PASS_AFTER_NEXT   "Peepholes"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! # testUndefined
//! METHOD            "optimize_istrue.ETSGLOBAL::__noinline__testUndefined"
//! PASS_AFTER        "Inline"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Inline"
//! PASS_AFTER_NEXT   "Peepholes"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! # testNaN
//! METHOD            "optimize_istrue.ETSGLOBAL::testNaN"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Double\.value/
//! INST_NOT          "Compare EQ f64"
//! INST_NOT          "Compare NE f64"
//! INST_NOT          "And "
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Double\.value/
//! INST_NEXT         "Compare EQ f64"
//! INST_NEXT         "Compare NE f64"
//! INST_NEXT         "And "
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testSignedNaN"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Double\.value/
//! INST_NOT          "Compare EQ f64"
//! INST_NOT          "Compare NE f64"
//! INST_NOT          "And "
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Double\.value/
//! INST_NEXT         "Compare EQ f64"
//! INST_NEXT         "Compare NE f64"
//! INST_NEXT         "And "
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! # testBigInt
//! METHOD            "optimize_istrue.ETSGLOBAL::testBigIntFalse"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.BigInt\.sign/
//! INST_NOT          "Compare NE i32"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.BigInt\.sign/
//! INST_NEXT         "Compare NE i32"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testBigIntTrue"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.BigInt\.sign/
//! INST_NOT          "Compare NE i32"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.BigInt\.sign/
//! INST_NEXT         "Compare NE i32"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::__noinline__testBigIntTrue2"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.BigInt\.sign/
//! INST_COUNT        "Compare NE i32", 0
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.BigInt\.sign/
//! INST_NEXT         "Compare NE i32"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! INST_COUNT        "Compare NE i32", 1
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! # testFoo
//! METHOD            "optimize_istrue.ETSGLOBAL::testFooTrue"
//! PASS_AFTER        "IfMerging"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "IfMerging"
//! PASS_AFTER_NEXT   "Peepholes"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testFooFalse"
//! PASS_AFTER        "IfMerging"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "IfMerging"
//! PASS_AFTER_NEXT   "Peepholes"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! # testEnum
//! METHOD            "optimize_istrue.ETSGLOBAL::testEnumFalse"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /CallStatic.*Color::valueOf/
//! INST_NOT          "Compare NE i32"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /CallStatic.*Color::valueOf/
//! INST_NEXT         "Compare NE i32"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testEnumTrue1"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /CallStatic.*Color::valueOf/
//! INST_NOT          "Compare NE i32"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /CallStatic.*Color::valueOf/
//! INST_NEXT         "Compare NE i32"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testEnumTrue2"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /CallStatic.*Color::valueOf/
//! INST_NOT          "Compare NE i32"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /CallStatic.*Color::valueOf/
//! INST_NEXT         "Compare NE i32"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testEnumTrue3"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /CallStatic.*Color::valueOf/
//! INST_NOT          "Compare NE i32"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /CallStatic.*Color::valueOf/
//! INST_NEXT         "Compare NE i32"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testEnum64False"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /CallStatic.*Color64::valueOf/
//! INST_NOT          "Compare NE i64"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /CallStatic.*Color64::valueOf/
//! INST_NEXT         "Compare NE i64"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testEnum64True1"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /CallStatic.*Color64::valueOf/
//! INST_NOT          "Compare NE i64"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /CallStatic.*Color64::valueOf/
//! INST_NEXT         "Compare NE i64"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testEnum64True2"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /CallStatic.*Color64::valueOf/
//! INST_NOT          "Compare NE i64"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /CallStatic.*Color64::valueOf/
//! INST_NEXT         "Compare NE i64"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testEnum64True3"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /CallStatic.*Color64::valueOf/
//! INST_NOT          "Compare NE i64"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /CallStatic.*Color64::valueOf/
//! INST_NEXT         "Compare NE i64"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testEnumStringFalse"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /CallStatic.*ColorString::valueOf/
//! INST_NOT          "LenArray"
//! INST_NOT          "Compare NE i32"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /CallStatic.*ColorString::valueOf/
//! INST_NEXT         "LenArray"
//! INST_NEXT         "Compare NE i32"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testEnumStringTrue"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /CallStatic.*ColorString::valueOf/
//! INST_NOT          "LenArray"
//! INST_NOT          "Compare NE i32"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /CallStatic.*ColorString::valueOf/
//! INST_NEXT         "LenArray"
//! INST_NEXT         "Compare NE i32"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testNonConstString"
//! PASS_BEFORE       "Peepholes"
//! INST              "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER        "Peepholes"
//! INST              "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER        "Codegen"
//! INST              "Intrinsic.CompilerEtsIstrue"
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testObject"
//! PASS_BEFORE       "Peepholes"
//! INST              "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER        "Peepholes"
//! INST              "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER        "Codegen"
//! INST              "Intrinsic.CompilerEtsIstrue"
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testMap"
//! PASS_BEFORE       "Peepholes"
//! INST              "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER        "Peepholes"
//! INST              "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER        "Codegen"
//! INST              "Intrinsic.CompilerEtsIstrue"
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testCheck"
//! PASS_BEFORE       "Peepholes"
//! INST              "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER        "Peepholes"
//! INST              "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER        "Codegen"
//! INST              "Intrinsic.CompilerEtsIstrue"
//!

//! CHECKER           JIT: Checking IsTrue optimization
//! RUN               force_jit: true, options: "--compiler-regex='.*test.*'", entry: "optimize_istrue.ETSGLOBAL::main"
//!
//! # testByte
//! METHOD            "optimize_istrue.ETSGLOBAL::testByteFalse"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Byte\.value/
//! INST_NOT          "Compare NE i8"
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Byte\.value/
//! INST_NEXT         "Compare NE i8"
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testByteTrue"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Byte\.value/
//! INST_NOT          "Compare NE i8"
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Byte\.value/
//! INST_NEXT         "Compare NE i8"
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! # testChar
//! METHOD            "optimize_istrue.ETSGLOBAL::testCharFalse"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Char\.value/
//! INST_NOT          "Compare NE u16"
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Char\.value/
//! INST_NEXT         "Compare NE u16"
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testCharTrue"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Char\.value/
//! INST_NOT          "Compare NE u16"
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Char\.value/
//! INST_NEXT         "Compare NE u16"
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! # testShort
//! METHOD            "optimize_istrue.ETSGLOBAL::testShortFalse"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Short\.value/
//! INST_NOT          "Compare NE i16"
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Short\.value/
//! INST_NEXT         "Compare NE i16"
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testShortTrue"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Short\.value/
//! INST_NOT          "Compare NE i16"
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Short\.value/
//! INST_NEXT         "Compare NE i16"
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! # testInt
//! METHOD            "optimize_istrue.ETSGLOBAL::testIntFalse"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Int\.value/
//! INST_NOT          "Compare NE i32"
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Int\.value/
//! INST_NEXT         "Compare NE i32"
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testIntTrue"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Int\.value/
//! INST_NOT          "Compare NE i32"
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Int\.value/
//! INST_NEXT         "Compare NE i32"
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! # testLong
//! METHOD            "optimize_istrue.ETSGLOBAL::testLongFalse"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Long\.value/
//! INST_NOT          "Compare NE i64"
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Long\.value/
//! INST_NEXT         "Compare NE i64"
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testLongTrue"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Long\.value/
//! INST_NOT          "Compare NE i64"
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Long\.value/
//! INST_NEXT         "Compare NE i64"
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! # testFloat
//! METHOD            "optimize_istrue.ETSGLOBAL::testFloatPositiveInfinity"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Float\.value/
//! INST_NOT          "Compare EQ f32"
//! INST_NOT          "Compare NE f32"
//! INST_NOT          "And "
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Float\.value/
//! INST_NEXT         "Compare EQ f32"
//! INST_NEXT         "Compare NE f32"
//! INST_NEXT         "And "
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testFloatNegativeInfinity"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Float\.value/
//! INST_NOT          "Compare EQ f32"
//! INST_NOT          "Compare NE f32"
//! INST_NOT          "And "
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Float\.value/
//! INST_NEXT         "Compare EQ f32"
//! INST_NEXT         "Compare NE f32"
//! INST_NEXT         "And "
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testFloatNaN"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Float\.value/
//! INST_NOT          "Compare EQ f32"
//! INST_NOT          "Compare NE f32"
//! INST_NOT          "And "
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Float\.value/
//! INST_NEXT         "Compare EQ f32"
//! INST_NEXT         "Compare NE f32"
//! INST_NEXT         "And "
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testSignedFloatNaN"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Float\.value/
//! INST_NOT          "Compare EQ f32"
//! INST_NOT          "Compare NE f32"
//! INST_NOT          "And "
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Float\.value/
//! INST_NEXT         "Compare EQ f32"
//! INST_NEXT         "Compare NE f32"
//! INST_NEXT         "And "
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testFloatFalse"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Float\.value/
//! INST_NOT          "Compare EQ f32"
//! INST_NOT          "Compare NE f32"
//! INST_NOT          "And "
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Float\.value/
//! INST_NEXT         "Compare EQ f32"
//! INST_NEXT         "Compare NE f32"
//! INST_NEXT         "And "
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testFloatTrue"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Float\.value/
//! INST_NOT          "Compare EQ f32"
//! INST_NOT          "Compare NE f32"
//! INST_NOT          "And "
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Float\.value/
//! INST_NEXT         "Compare EQ f32"
//! INST_NEXT         "Compare NE f32"
//! INST_NEXT         "And "
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! # testDouble
//! METHOD            "optimize_istrue.ETSGLOBAL::testDoublePositiveInfinity"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Double\.value/
//! INST_NOT          "Compare EQ f64"
//! INST_NOT          "Compare NE f64"
//! INST_NOT          "And "
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Double\.value/
//! INST_NEXT         "Compare EQ f64"
//! INST_NEXT         "Compare NE f64"
//! INST_NEXT         "And "
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testDoubleNegativeInfinity"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Double\.value/
//! INST_NOT          "Compare EQ f64"
//! INST_NOT          "Compare NE f64"
//! INST_NOT          "And "
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Double\.value/
//! INST_NEXT         "Compare EQ f64"
//! INST_NEXT         "Compare NE f64"
//! INST_NEXT         "And "
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testDoubleNaN"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Double\.value/
//! INST_NOT          "Compare EQ f64"
//! INST_NOT          "Compare NE f64"
//! INST_NOT          "And "
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Double\.value/
//! INST_NEXT         "Compare EQ f64"
//! INST_NEXT         "Compare NE f64"
//! INST_NEXT         "And "
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testSignedDoubleNaN"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Double\.value/
//! INST_NOT          "Compare EQ f64"
//! INST_NOT          "Compare NE f64"
//! INST_NOT          "And "
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Double\.value/
//! INST_NEXT         "Compare EQ f64"
//! INST_NEXT         "Compare NE f64"
//! INST_NEXT         "And "
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testDoubleFalse"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Double\.value/
//! INST_NOT          "Compare EQ f64"
//! INST_NOT          "Compare NE f64"
//! INST_NOT          "And "
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Double\.value/
//! INST_NEXT         "Compare EQ f64"
//! INST_NEXT         "Compare NE f64"
//! INST_NEXT         "And "
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testDoubleTrue"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Double\.value/
//! INST_NOT          "Compare EQ f64"
//! INST_NOT          "Compare NE f64"
//! INST_NOT          "And "
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Double\.value/
//! INST_NEXT         "Compare EQ f64"
//! INST_NEXT         "Compare NE f64"
//! INST_NEXT         "And "
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! # testString
//! METHOD            "optimize_istrue.ETSGLOBAL::testStringFalse"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          "LenArray"
//! INST_NOT          "Compare NE i32"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              "LenArray"
//! INST_NEXT         "Compare NE i32"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testStringTrue"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          "LenArray"
//! INST_NOT          "Compare NE i32"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              "LenArray"
//! INST_NEXT         "Compare NE i32"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! # testBoolean
//! METHOD            "optimize_istrue.ETSGLOBAL::testBooleanFalse"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Boolean\.value/
//! INST_NOT          "Compare NE b"
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Boolean\.value/
//! INST_NEXT         "Compare NE b"
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testBooleanTrue"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Boolean\.value/
//! INST_NOT          "Compare NE b"
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Boolean\.value/
//! INST_NEXT         "Compare NE b"
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! # testNull
//! METHOD            "optimize_istrue.ETSGLOBAL::__noinline__testNull"
//! PASS_AFTER        "Inline"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Inline"
//! PASS_AFTER_NEXT   "Peepholes"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! # testUndefined
//! METHOD            "optimize_istrue.ETSGLOBAL::__noinline__testUndefined"
//! PASS_AFTER        "Inline"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Inline"
//! PASS_AFTER_NEXT   "Peepholes"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! # testNaN
//! METHOD            "optimize_istrue.ETSGLOBAL::testNaN"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Double\.value/
//! INST_NOT          "Compare EQ f64"
//! INST_NOT          "Compare NE f64"
//! INST_NOT          "And "
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Double\.value/
//! INST_NEXT         "Compare EQ f64"
//! INST_NEXT         "Compare NE f64"
//! INST_NEXT         "And "
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testSignedNaN"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.Double\.value/
//! INST_NOT          "Compare EQ f64"
//! INST_NOT          "Compare NE f64"
//! INST_NOT          "And "
//! INST_NOT          "Phi"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.Double\.value/
//! INST_NEXT         "Compare EQ f64"
//! INST_NEXT         "Compare NE f64"
//! INST_NEXT         "And "
//! INST_NEXT         "Phi"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! # testBigInt
//! METHOD            "optimize_istrue.ETSGLOBAL::testBigIntFalse"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.BigInt\.sign/
//! INST_NOT          "Compare NE i32"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.BigInt\.sign/
//! INST_NEXT         "Compare NE i32"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testBigIntTrue"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.BigInt\.sign/
//! INST_NOT          "Compare NE i32"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.BigInt\.sign/
//! INST_NEXT         "Compare NE i32"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::__noinline__testBigIntTrue2"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /LoadObject.*std\.core\.BigInt\.sign/
//! INST_COUNT        "Compare NE i32", 0
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /LoadObject.*std\.core\.BigInt\.sign/
//! INST_NEXT         "Compare NE i32"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! INST_COUNT        "Compare NE i32", 1
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! # testFoo
//! METHOD            "optimize_istrue.ETSGLOBAL::testFooTrue"
//! PASS_AFTER        "IfMerging"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "IfMerging"
//! PASS_AFTER_NEXT   "Peepholes"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testFooFalse"
//! PASS_AFTER        "IfMerging"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "IfMerging"
//! PASS_AFTER_NEXT   "Peepholes"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! # testEnum
//! METHOD            "optimize_istrue.ETSGLOBAL::testEnumFalse"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /CallStatic.*Color::valueOf/
//! INST_NOT          "Compare NE i32"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /CallStatic.*Color::valueOf/
//! INST_NEXT         "Compare NE i32"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testEnumTrue1"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /CallStatic.*Color::valueOf/
//! INST_NOT          "Compare NE i32"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /CallStatic.*Color::valueOf/
//! INST_NEXT         "Compare NE i32"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testEnumTrue2"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /CallStatic.*Color::valueOf/
//! INST_NOT          "Compare NE i32"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /CallStatic.*Color::valueOf/
//! INST_NEXT         "Compare NE i32"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testEnumTrue3"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /CallStatic.*Color::valueOf/
//! INST_NOT          "Compare NE i32"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /CallStatic.*Color::valueOf/
//! INST_NEXT         "Compare NE i32"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testEnum64False"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /CallStatic.*Color64::valueOf/
//! INST_NOT          "Compare NE i64"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /CallStatic.*Color64::valueOf/
//! INST_NEXT         "Compare NE i64"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testEnum64True1"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /CallStatic.*Color64::valueOf/
//! INST_NOT          "Compare NE i64"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /CallStatic.*Color64::valueOf/
//! INST_NEXT         "Compare NE i64"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testEnum64True2"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /CallStatic.*Color64::valueOf/
//! INST_NOT          "Compare NE i64"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /CallStatic.*Color64::valueOf/
//! INST_NEXT         "Compare NE i64"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testEnum64True3"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /CallStatic.*Color64::valueOf/
//! INST_NOT          "Compare NE i64"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /CallStatic.*Color64::valueOf/
//! INST_NEXT         "Compare NE i64"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testEnumStringFalse"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /CallStatic.*ColorString::valueOf/
//! INST_NOT          "LenArray"
//! INST_NOT          "Compare NE i32"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /CallStatic.*ColorString::valueOf/
//! INST_NEXT         "LenArray"
//! INST_NEXT         "Compare NE i32"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testEnumStringTrue"
//! PASS_BEFORE       "Peepholes"
//! INST_NOT          /CallStatic.*ColorString::valueOf/
//! INST_NOT          "LenArray"
//! INST_NOT          "Compare NE i32"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 1
//! PASS_AFTER        "Peepholes"
//! INST              /CallStatic.*ColorString::valueOf/
//! INST_NEXT         "LenArray"
//! INST_NEXT         "Compare NE i32"
//! INST_NEXT         "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        "Intrinsic.CompilerEtsIstrue", 0
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testNonConstString"
//! PASS_BEFORE       "Peepholes"
//! INST              "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER        "Peepholes"
//! INST              "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER        "Codegen"
//! INST              "Intrinsic.CompilerEtsIstrue"
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testObject"
//! PASS_BEFORE       "Peepholes"
//! INST              "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER        "Peepholes"
//! INST              "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER        "Codegen"
//! INST              "Intrinsic.CompilerEtsIstrue"
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testMap"
//! PASS_BEFORE       "Peepholes"
//! INST              "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER        "Peepholes"
//! INST              "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER        "Codegen"
//! INST              "Intrinsic.CompilerEtsIstrue"
//!
//! METHOD            "optimize_istrue.ETSGLOBAL::testCheck"
//! PASS_BEFORE       "Peepholes"
//! INST              "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER        "Peepholes"
//! INST              "Intrinsic.CompilerEtsIstrue"
//! PASS_AFTER        "Codegen"
//! INST              "Intrinsic.CompilerEtsIstrue"
//!

function __noinline__dummy(o: Object) {
}

function __noinline__dummyReturn(): Int {
    return 1;
}

function testByteFalse() {
    let x: byte = 0;
    return !!((new Byte(x)) as Object);
}

function testByteTrue() {
    let x: byte = -100;
    return !!((new Byte(x)) as Object);
}

function testCharFalse() {
    let x: char = Int.toChar(0);
    return !!((new Char(x)) as Object);
}

function testCharTrue() {
    let x: char = Int.toChar(100);
    return !!((new Char(x)) as Object);
}

function testShortFalse() {
    let x: short = 0;
    return !!((new Short(x)) as Object);
}

function testShortTrue() {
    let x: short = -100;
    return !!((new Short(x)) as Object);
}

function testIntFalse() {
    return !!((new Int(0)) as Object);
}

function testIntTrue() {
    return !!((new Int(-100)) as Object);
}

function testLongFalse() {
    return !!((new Long(0)) as Object);
}

function testLongTrue() {
    return !!((new Long(-100)) as Object);
}

function testFloatPositiveInfinity() {
    return !!((new Float(Float.POSITIVE_INFINITY)) as Object);
}

function testFloatNegativeInfinity() {
    return !!((new Float(Float.NEGATIVE_INFINITY)) as Object);
}

function testFloatNaN() {
    return !!((new Float(Float.NaN)) as Object);
}

function testSignedFloatNaN() {
    return !!((new Float(-Float.NaN)) as Object);
}

function testFloatFalse() {
    return !!((new Float(0.0)) as Object);
}

function testFloatTrue() {
    return !!((new Float(-100.5)) as Object);
}

function testDoublePositiveInfinity() {
    return !!((new Double(Double.POSITIVE_INFINITY)) as Object);
}

function testDoubleNegativeInfinity() {
    return !!((new Double(Double.NEGATIVE_INFINITY)) as Object);
}

function testDoubleNaN() {
    return !!((new Double(Double.NaN)) as Object);
}

function testSignedDoubleNaN() {
    return !!((new Double(-Double.NaN)) as Object);
}

function testDoubleFalse() {
    return !!((new Double(0.0)) as Object);
}

function testDoubleTrue() {
    return !!((new Double(-100.5)) as Object);
}

function testStringFalse() {
    return !!(("") as Object);
}

function testStringTrue() {
    return !!(("test") as Object);
}

function testBooleanFalse() {
    return !!((new Boolean(false)) as Object);
}

function testBooleanTrue() {
    return !!((new Boolean(true)) as Object);
}

function testFooNull(v: Object | null | undefined) {
    return !!(v);
}

function __noinline__testNull() {
    return testFooNull(null);
}

function testFooUndefined(v: Object | null | undefined) {
    return !!(v);
}

function __noinline__testUndefined() {
    return testFooUndefined(undefined);
}

function testNaN() {
    return !!(NaN as Object);
}

function testSignedNaN() {
    return !!(-NaN as Object);
}

function testBigIntFalse() {
    return !!((new bigint(0)) as Object);
}

function testBigIntTrue() {
    return !!((new bigint(1)) as Object);
}

function testBigIntFoo(v: Object | null | undefined) {
    return !!(v);
}

function __noinline__testBigIntTrue2() {
    let a: bigint | null = 0n;
    return testBigIntFoo(!a);
}

class Klass {
    public f: number;

    constructor(f: number) {
        this.f = f;
    }
}

function foo(n: number): Klass | undefined {
    if (n == 0) {
        return undefined;
    }
    return new Klass(n);
}

function testFooTrue() {
    let x: boolean = !!(foo(1));
    __noinline__dummy(x);
    return x;
}

function testFooFalse() {
    let x: boolean = !!(foo(0));
    __noinline__dummy(x);
    return x;
}

enum Color {
    Red,
    Green,
    Blue = 2147483647,
    Yellow = -2147483648,
}

enum Color64 {
    Red,
    Green,
    Blue = 9223372036854775807,
    Yellow = -9223372036854775808,
}

enum ColorString {
    Red = "",
    Green = "Green"
}

function testEnumFalse() {
    let x: boolean = !!(Color.Red as Object);
    __noinline__dummy(x);
    for (let i = 0; i < 5000; i++) {
        let t: Int = __noinline__dummyReturn()
    }
    return x;
}

function testEnumTrue1() {
    let x: boolean = !!(Color.Green as Object);
    __noinline__dummy(x);
    for (let i = 0; i < 5000; i++) {
        let t: Int = __noinline__dummyReturn()
    }
    return x;
}

function testEnumTrue2() {
    let x: boolean = !!(Color.Blue as Object);
    __noinline__dummy(x);
    for (let i = 0; i < 5000; i++) {
        let t: Int = __noinline__dummyReturn()
    }
    return x;
}

function testEnumTrue3() {
    let x: boolean = !!(Color.Yellow as Object);
    __noinline__dummy(x);
    for (let i = 0; i < 5000; i++) {
        let t: Int = __noinline__dummyReturn()
    }
    return x;
}

function testEnum64False() {
    let x: boolean = !!(Color64.Red as Object);
    __noinline__dummy(x);
    for (let i = 0; i < 5000; i++) {
        let t: Int = __noinline__dummyReturn()
    }
    return x;
}

function testEnum64True1() {
    let x: boolean = !!(Color64.Green as Object);
    __noinline__dummy(x);
    return x;
}

function testEnum64True2() {
    let x: boolean = !!(Color64.Blue as Object);
    __noinline__dummy(x);
    for (let i = 0; i < 5000; i++) {
        let t: Int = __noinline__dummyReturn()
    }
    return x;
}

function testEnum64True3() {
    let x: boolean = !!(Color64.Yellow as Object);
    __noinline__dummy(x);
    for (let i = 0; i < 5000; i++) {
        let t: Int = __noinline__dummyReturn()
    }
    return x;
}

function testEnumStringFalse() {
    let x: boolean = !!(ColorString.Red as Object);
    __noinline__dummy(x);
    for (let i = 0; i < 5000; i++) {
        let t: Int = __noinline__dummyReturn()
    }
    return x;
}

function testEnumStringTrue() {
    let x: boolean = !!(ColorString.Green as Object);
    __noinline__dummy(x);
    for (let i = 0; i < 5000; i++) {
        let t: Int = __noinline__dummyReturn()
    }
    return x;
}

function testNonConstString() {
    let str = "test".substring(1);
    return !!(str as Object);
}

function testObject(o: Object): boolean {
    return !!o;
}

function testMap(map: Map<int, int>): boolean {
    return !!map;
}

function testCheck(v: Any) {
    return !!v;
}

function main() {
    arktest.assertFalse(testByteFalse());
    arktest.assertTrue(testByteTrue());
    arktest.assertFalse(testShortFalse());
    arktest.assertTrue(testShortTrue());
    arktest.assertFalse(testIntFalse());
    arktest.assertTrue(testIntTrue());
    arktest.assertFalse(testLongFalse());
    arktest.assertTrue(testLongTrue());
    arktest.assertTrue(testFloatPositiveInfinity());
    arktest.assertTrue(testFloatNegativeInfinity());
    arktest.assertFalse(testFloatNaN());
    arktest.assertFalse(testSignedFloatNaN());
    arktest.assertFalse(testFloatFalse());
    arktest.assertTrue(testFloatTrue());
    arktest.assertTrue(testDoublePositiveInfinity());
    arktest.assertTrue(testDoubleNegativeInfinity());
    arktest.assertFalse(testDoubleNaN());
    arktest.assertFalse(testSignedDoubleNaN());
    arktest.assertFalse(testDoubleFalse());
    arktest.assertTrue(testDoubleTrue());
    arktest.assertFalse(testCharFalse());
    arktest.assertTrue(testCharTrue());
    arktest.assertFalse(testStringFalse());
    arktest.assertTrue(testStringTrue());
    arktest.assertFalse(testBooleanFalse());
    arktest.assertTrue(testBooleanTrue());
    arktest.assertFalse(testNaN());
    arktest.assertFalse(testSignedNaN());
    arktest.assertFalse(__noinline__testNull());
    arktest.assertFalse(__noinline__testUndefined());
    arktest.assertFalse(testBigIntFalse());
    arktest.assertTrue(testBigIntTrue());
    arktest.assertTrue(__noinline__testBigIntTrue2());
    arktest.assertTrue(testFooTrue());
    arktest.assertFalse(testFooFalse());
    arktest.assertFalse(testEnumFalse());
    arktest.assertTrue(testEnumTrue1());
    arktest.assertTrue(testEnumTrue2());
    arktest.assertTrue(testEnumTrue3());
    arktest.assertFalse(testEnum64False());
    arktest.assertTrue(testEnum64True1());
    arktest.assertTrue(testEnum64True2());
    arktest.assertTrue(testEnum64True3());
    arktest.assertFalse(testEnumStringFalse());
    arktest.assertTrue(testEnumStringTrue());

    // Negative cases.
    arktest.assertTrue(testNonConstString());

    let o = new Object();
    arktest.assertTrue(testObject(o));

    let m = new Map<int, int>();
    arktest.assertTrue(testMap(m));

    arktest.assertTrue(testCheck(new Byte(1)));
    arktest.assertFalse(testCheck(new Byte(0)));

    return 0;
}
