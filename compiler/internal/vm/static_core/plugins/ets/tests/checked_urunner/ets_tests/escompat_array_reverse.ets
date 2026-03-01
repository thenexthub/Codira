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

function check_array<T>(src: Array<T>, exp: Array<T>) {
    arktest.assertEQ(src.length, exp.length, "reverse(): bad array len")
    for (let i = 0; i < exp.length; i++) {
        arktest.assertEQ(src[i], exp[i], "reverse(): unexpected element: i: " + i)
    }
}

//! DEFINE_FRONTEND_OPTIONS            DEFAULT ARGS
//! RUN_FRONTEND            options: "--ets-strings-concat=false"

//! CHECKER       INT: ArrayInt reverse
//! RUN           options: "--compiler-enable-jit=false", entry: "escompat_array_reverse.ETSGLOBAL::test_array_int_reverse"

//! CHECKER       JIT: ArrayInt reverse
//! RUN           force_jit: true, options: "--compiler-regex=.*test_array_int_reverse.*", entry: "escompat_array_reverse.ETSGLOBAL::test_array_int_reverse"
//! METHOD        "escompat_array_reverse.ETSGLOBAL::test_array_int_reverse"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      /Intrinsic.EscompatArrayReverse/
//! INST          /CallStatic .*std.core.Array::reverse/
//! PASS_AFTER    "Inline"
//! INST          /CallStatic.Inlined.*std.core.Array::reverse/
//! INST          /Intrinsic.EscompatArrayReverse/
//! ASM_METHOD    "escompat_array_reverse.ETSGLOBAL::test_array_int_reverse"
//! ASM_INST      "Intrinsic.EscompatArrayReverse"

//! CHECKER       AOT: ArrayInt reverse
//! RUN_PAOC      options: "--compiler-regex=.*test_array_int_reverse.* --compiler-inline-external-methods-aot=false", entry: "escompat_array_reverse.ETSGLOBAL::test_array_int_reverse"
//! METHOD        "escompat_array_reverse.ETSGLOBAL::test_array_int_reverse"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      /Intrinsic.EscompatArrayReverse/
//! INST          /CallStatic .*std.core.Array::reverse/
//! PASS_AFTER    "Inline"
//! EVENT         /Inline,escompat_array_reverse.ETSGLOBAL::test_array_int_reverse,std.core.Array::reverse,.*,STATIC,SKIP_EXTERNAL/
//! INST_NOT      /CallStatic.Inlined.*std.core.Array::reverse/
//! INST_NOT      /Intrinsic.EscompatArrayReverse/

//! CHECKER       AOT PGO: ArrayInt reverse
//! RUN_PGO_PROF  entry: "escompat_array_reverse.ETSGLOBAL::test_array_int_reverse"
//! RUN_PGO_PAOC  options: "--compiler-regex=.*test_array_int_reverse.* --compiler-inline-external-methods-aot=true"
//! METHOD        "escompat_array_reverse.ETSGLOBAL::test_array_int_reverse"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      /Intrinsic.EscompatArrayReverse/
//! INST          /CallStatic .*std.core.Array::reverse/
//! PASS_AFTER    "Inline"
//! EVENT         /Inline,escompat_array_reverse.ETSGLOBAL::test_array_int_reverse,std.core.Array::reverse,.*,STATIC,SUCCESS/
//! INST          /CallStatic.Inlined.*std.core.Array::reverse/
//! INST          /Intrinsic.EscompatArrayReverse/
//! ASM_METHOD    "escompat_array_reverse.ETSGLOBAL::test_array_int_reverse"
//! ASM_INST      "Intrinsic.EscompatArrayReverse"

function test_array_int_reverse() {
    let inp: Array<int> = [1, 2, 3, 4, 5, 6, 7];
    let exp: Array<int> = [7, 6, 5, 4, 3, 2, 1];
    check_array<int>(inp.reverse(), exp);
}

//! CHECKER       INT: ArrayUnion reverse
//! RUN           options: "--compiler-enable-jit=false", entry: "escompat_array_reverse.ETSGLOBAL::test_array_union_reverse"

//! CHECKER       JIT: ArrayUnion reverse
//! RUN           force_jit: true, options: "--compiler-regex=.*test_array_union_reverse.*", entry: "escompat_array_reverse.ETSGLOBAL::test_array_union_reverse"
//! METHOD        "escompat_array_reverse.ETSGLOBAL::test_array_union_reverse"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      /Intrinsic.EscompatArrayReverse/
//! INST          /CallStatic .*std.core.Array::reverse/
//! PASS_AFTER    "Inline"
//! INST          /CallStatic.Inlined.*std.core.Array::reverse/
//! INST          /Intrinsic.EscompatArrayReverse/
//! ASM_METHOD    "escompat_array_reverse.ETSGLOBAL::test_array_union_reverse"
//! ASM_INST      "Intrinsic.EscompatArrayReverse"

//! CHECKER       AOT: ArrayUnion reverse
//! RUN_PAOC      options: "--compiler-regex=.*test_array_union_reverse.* --compiler-inline-external-methods-aot=false", entry: "escompat_array_reverse.ETSGLOBAL::test_array_union_reverse"
//! METHOD        "escompat_array_reverse.ETSGLOBAL::test_array_union_reverse"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      /Intrinsic.EscompatArrayReverse/
//! INST          /CallStatic .*std.core.Array::reverse/
//! PASS_AFTER    "Inline"
//! EVENT         /Inline,escompat_array_reverse.ETSGLOBAL::test_array_union_reverse,std.core.Array::reverse,.*,STATIC,SKIP_EXTERNAL/
//! INST_NOT      /CallStatic.Inlined.*std.core.Array::reverse/
//! INST_NOT      /Intrinsic.EscompatArrayReverse/

//! CHECKER       AOT PGO: ArrayUnion reverse
//! RUN_PGO_PROF  entry: "escompat_array_reverse.ETSGLOBAL::test_array_union_reverse"
//! RUN_PGO_PAOC  options: "--compiler-regex=.*test_array_union_reverse.* --compiler-inline-external-methods-aot=true"
//! METHOD        "escompat_array_reverse.ETSGLOBAL::test_array_union_reverse"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      /Intrinsic.EscompatArrayReverse/
//! INST          /CallStatic .*std.core.Array::reverse/
//! PASS_AFTER    "Inline"
//! EVENT         /Inline,escompat_array_reverse.ETSGLOBAL::test_array_union_reverse,std.core.Array::reverse,.*,STATIC,SUCCESS/
//! INST          /CallStatic.Inlined.*std.core.Array::reverse/
//! INST          /Intrinsic.EscompatArrayReverse/
//! ASM_METHOD    "escompat_array_reverse.ETSGLOBAL::test_array_union_reverse"
//! ASM_INST      "Intrinsic.EscompatArrayReverse"

function test_array_union_reverse() {
    let inp: Array<char | number | string | boolean> = [1, c'b', "abc", c'4', 5, "6", false];
    let exp: Array<char | number | string | boolean> = [false, "6", 5, c'4', "abc", c'b', 1];
    check_array<char | number | string | boolean>(inp.reverse(), exp);
}
