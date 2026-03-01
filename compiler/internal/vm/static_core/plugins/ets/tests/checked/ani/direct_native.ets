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

export class NativeModule {
    static {
        loadLibrary("checked_test_direct_native")
    }

    @ani.unsafe.Direct
    static native direct(fake: int, i0: byte, i1: short, i2: int, i3: long, f0: float, f1: double, f2: double,
        i4: int, i5: int, i6: int, i7: long, i8: int, i9: short, i10: int, i11: int, i12: int, i13: int, i14: int,
        f3: float, f4: float, f5: double, f6: double, f7: double, f8: double, f9: double, f10: double, f11: double, f12: double, f13: double, f14: double): int;
}

function test(i0: byte, i1: short, i2: int, i3: long, f0: float, f1: double, f2: double,
              i4: int, i5: int, i6: int, i7: long, i8: int, i9: short, i10: int, i11: int, i12: int, i13: int, i14: int,
              f3: float, f4: float, f5: double, f6: double, f7: double, f8: double, f9: double, f10: double, f11: double, f12: double, f13: double, f14: double): int {
    let result = NativeModule.direct(-1, i0, i1, i2, i3, f0, f1, f2,
        i4, i5, i6, i7, i8, i9, i10, i11, i12, i13, i14,
        f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14)
    return result
}

//! CHECKER       Direct ANI call optimization, JIT
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, options: "--compiler-hotness-threshold=1 --compiler-regex='direct_native.ETSGLOBAL::test.*'", entry: "direct_native.ETSGLOBAL::main"
//! METHOD        "direct_native.ETSGLOBAL::test"
//! PASS_AFTER    "IrBuilder"
//! INST_COUNT    /CallStatic/, 1
//! PASS_AFTER    "NativeCallOptimization"
//! INST_COUNT    /CallStatic/, 0
//! INST          /Intrinsic\.CompilerGetNativeMethod/
//! INST_NEXT     /Intrinsic\.CompilerGetMethodNativePointer/
//! INST_NEXT     /CallNative/

//! CHECKER       Direct ANI call optimization, AOT
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC       options: "--compiler-regex='direct_native.ETSGLOBAL::test.*'"
//! METHOD        "direct_native.ETSGLOBAL::test"
//! PASS_AFTER    "IrBuilder"
//! INST_COUNT    /CallStatic/, 1
//! PASS_AFTER    "NativeCallOptimization"
//! INST_COUNT    /CallStatic/, 0
//! INST          /Intrinsic\.CompilerGetNativeMethod/
//! INST_NEXT     /Intrinsic\.CompilerGetMethodNativePointer/
//! INST_NEXT     /CallNative/
//! RUN           entry: "direct_native.ETSGLOBAL::main"

function main(): int {
    let res = 0;
    for (let i = 0; i < 100; ++i) {
        res = test(0 as byte, 1 as short, 2, 3 as long, 0.0f, 1.0, 2.0,
          4, 5, 6, 7 as long, 8, 9 as short, 10, 11, 12, 13, 14,
          3.0f, 4.0f, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0);
        if (res != 0) {
            return res;
        }
    }
    return res;
}
