/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
//! RUN_FRONTEND            options: "--ets-strings-concat=false --bco-compiler --compiler-simplify-string-builder=false"

//! CHECKER       AOT IR Builder, check String concatenation loop
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*concat_loop[0-9]*' --compiler-inlining=false"
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concat_loop0"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST          /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concat_loop1"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST          /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concat_loop2"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /head, loop/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST          /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /head, loop/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concat_loop3"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST          /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concat_loop4"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST          /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concat_loop5"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! PASS_AFTER    "ChecksElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,1
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concat_loop7"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,0
//! PASS_AFTER    "ChecksElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,1
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::equivalent_concat_loop7"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,4
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! PASS_AFTER    "ChecksElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,1
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concat_loop8"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /StringBuilder::<ctor>/,6
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,4
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,6
//! PASS_AFTER    "ChecksElimination"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,0
//! PASS_AFTER    "ChecksElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,4
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "Codegen"
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::nested_concat_loop0"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop 3/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST          /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 3/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 2/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 1/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::try_catch_concat_loop0"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,6
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,8
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,6
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbAppendInt/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! INST_COUNT    /Intrinsic.StdCoreStringConcat2/,3
//! INST_COUNT    /Intrinsic.StdCoreStringConcat4/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::try_catch_concat_loop1"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbAppendInt/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! INST_COUNT    /Intrinsic.StdCoreStringConcat2/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::try_catch_concat_loop2"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,3
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,3
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,0
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString /,2
//! INST_NOT      /Intrinsic.StdCoreSbAppendInt/
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::try_catch_concat_loop3"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbAppendInt/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! INST_COUNT    /Intrinsic.StdCoreStringConcat2/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::try_catch_concat_loop4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbAppendInt/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! INST_COUNT    /Intrinsic.StdCoreStringConcat2/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::try_catch_concat_loop5"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,5
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,5
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,5
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,0
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString /,2
//! INST_COUNT    /Intrinsic.StdCoreStringConcat2/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop0"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,4
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,4
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop1"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,3
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop2"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,3
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop3"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,3
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,3
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop5"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop6"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop7"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,0
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,0
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop8"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop9"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop10"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop 1, depth 1, bc: 0x00000017/
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,0
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,0
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 1, depth 1, bc: 0x00000017/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop11"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop 1, depth 1, bc: 0x00000017/
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,0
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,0
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 1, depth 1, bc: 0x00000017/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::switch_concat_loop0"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,6
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,4
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,6
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,4
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::switch_concat_loop1"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,6
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,4
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,6
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,4
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuse_concat_loop0"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString /,0
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuse_concat_loop1"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuse_concat_loop2"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuse_concat_loop3"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /StringBuilder::<ctor>/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuse_concat_loop4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,0
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString /,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuse_concat_loop5"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,0
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString /,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::args_concat_loop0"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,0
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendBool/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendChar/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendDouble/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,0
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendBool/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendChar/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendDouble/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//!
//! RUN           options: "--compiler-regex='.*concat_loop[0-9]*'", entry: "ets_string_concat_loop.ETSGLOBAL::main"

//! CHECKER       JIT IR Builder, check String concatenation loop
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, options: "--compiler-regex='.*concat_loop[0-9]+.*' --compiler-inlining=false", entry: "ets_string_concat_loop.ETSGLOBAL::main"
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concat_loop0"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST          /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concat_loop1"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST          /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concat_loop2"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /head, loop/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST          /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /head, loop/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concat_loop3"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST          /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concat_loop4"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST          /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concat_loop5"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! PASS_AFTER    "ChecksElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,1
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concat_loop7"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,0
//! PASS_AFTER    "ChecksElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,1
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::equivalent_concat_loop7"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,4
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! PASS_AFTER    "ChecksElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,1
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concat_loop8"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /StringBuilder::<ctor>/,6
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,4
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,6
//! PASS_AFTER    "ChecksElimination"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,0
//! PASS_AFTER    "ChecksElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,4
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "Codegen"
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::nested_concat_loop0"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop 3/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST          /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 3/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 2/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 1/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::try_catch_concat_loop0"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,6
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,8
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,6
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbAppendInt/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! INST_COUNT    /Intrinsic.StdCoreStringConcat2/,3
//! INST_COUNT    /Intrinsic.StdCoreStringConcat4/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::try_catch_concat_loop1"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbAppendInt/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! INST_COUNT    /Intrinsic.StdCoreStringConcat2/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::try_catch_concat_loop2"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,3
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,3
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,0
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString /,2
//! INST_NOT      /Intrinsic.StdCoreSbAppendInt/
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::try_catch_concat_loop3"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbAppendInt/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! INST_COUNT    /Intrinsic.StdCoreStringConcat2/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::try_catch_concat_loop4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbAppendInt/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! INST_COUNT    /Intrinsic.StdCoreStringConcat2/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::try_catch_concat_loop5"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,5
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,5
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,5
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,0
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString /,2
//! INST_COUNT    /Intrinsic.StdCoreStringConcat2/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop0"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,4
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,4
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop1"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,3
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop2"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,3
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop3"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,3
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,3
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop5"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop6"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop7"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,0
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,0
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop8"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop9"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop10"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop 1, depth 1, bc: 0x00000017/
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,0
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,0
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 1, depth 1, bc: 0x00000017/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop11"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop 1, depth 1, bc: 0x00000017/
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,0
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,0
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 1, depth 1, bc: 0x00000017/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::switch_concat_loop0"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,6
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,4
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,6
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,4
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::switch_concat_loop1"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,6
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,4
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,6
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,4
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuse_concat_loop0"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString /,0
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuse_concat_loop1"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuse_concat_loop2"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuse_concat_loop3"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /StringBuilder::<ctor>/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuse_concat_loop4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,0
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString /,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuse_concat_loop5"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,0
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString /,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::args_concat_loop0"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,0
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendBool/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendChar/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendDouble/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,0
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendBool/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendChar/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendDouble/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2

//! CHECKER       Invalid save states JIT (#I932LA/15833)
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, options: "--compiler-inlining=false --compiler-regex='.*bugfix_concat_loop1'", entry: "ets_string_concat_loop.ETSGLOBAL::main"
//!
//! METHOD        "bugfix_concat_loop1"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop 1/
//! INST          /StringBuilder::<ctor>/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 1/
//! INST_NOT      /StringBuilder::<ctor>/

//! CHECKER       Invalid save states AOT (#I932LA/15833)
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*bugfix_concat_loop1'"
//!
//! METHOD        "bugfix_concat_loop1"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop 1/
//! INST          /StringBuilder::<ctor>/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 1/
//! INST_NOT      /StringBuilder::<ctor>/
//!
//! RUN           options: "--compiler-regex='.*bugfix_concat_loop1'", entry: "ets_string_concat_loop.ETSGLOBAL::main"


//! DEFINE_FRONTEND_OPTIONS            RUN_BCO reuse .abc
//! RUN_BCO       options: "--ets-strings-concat=false --bco-compiler --compiler-simplify-string-builder=true"

//! CHECKER       BCO, check String concatenation loop
//! DEFINE_FRONTEND_OPTIONS            RUN_BCO reuse .abc
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concat_loop0"
//! PASS_AFTER    "BranchElimination"
//! IN_BLOCK      /loop/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /StringBuilder::append/,1
//! INST          /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST_NOT      /StringBuilder::append/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop/
//! INST          /StringBuilder::append/
//! INST_NOT      /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /StringBuilder::toString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concat_loop1"
//! PASS_AFTER    "BranchElimination"
//! IN_BLOCK      /loop/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /StringBuilder::append/,1
//! INST          /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST_NOT      /StringBuilder::append/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop/
//! INST          /StringBuilder::append/
//! INST_NOT      /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /StringBuilder::toString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concat_loop2"
//! PASS_AFTER    "BranchElimination"
//! IN_BLOCK      /head, loop/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /StringBuilder::append/,1
//! INST          /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST_NOT      /StringBuilder::append/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /head, loop/
//! INST          /StringBuilder::append/
//! INST_NOT      /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /StringBuilder::toString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concat_loop3"
//! PASS_AFTER    "BranchElimination"
//! IN_BLOCK      /loop/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /StringBuilder::append/,1
//! INST          /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST_NOT      /StringBuilder::append/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop/
//! INST          /StringBuilder::append/
//! INST_NOT      /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /StringBuilder::toString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concat_loop4"
//! PASS_AFTER    "BranchElimination"
//! IN_BLOCK      /loop/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /StringBuilder::append/,1
//! INST          /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST_NOT      /StringBuilder::append/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop/
//! INST          /StringBuilder::append/
//! INST_NOT      /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /StringBuilder::toString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concat_loop5"
//! PASS_AFTER    "BranchElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST_NOT      /StringBuilder::append/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop/
//! INST_COUNT    /StringBuilder::append/,2
//! INST_NOT      /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /StringBuilder::toString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concat_loop7"
//! PASS_AFTER    "BranchElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,0
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop/
//! INST_COUNT    /StringBuilder::append/,2
//! INST_NOT      /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::equivalent_concat_loop7"
//! PASS_AFTER    "BranchElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /StringBuilder::append/,4
//! INST_COUNT    /StringBuilder::toString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,1
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop/
//! INST_COUNT    /StringBuilder::append/,2
//! INST_NOT      /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concat_loop8"
//! PASS_AFTER    "BranchElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /StringBuilder::<ctor>/,6
//! INST_COUNT    /StringBuilder::append/,6
//! INST_COUNT    /StringBuilder::toString/,6
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /StringBuilder::append/,0
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop/
//! INST_COUNT    /StringBuilder::append/,6
//! INST_NOT      /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::toString/,2
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::nested_concat_loop0"
//! PASS_AFTER    "BranchElimination"
//! IN_BLOCK      /loop 3/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /StringBuilder::append/,1
//! INST          /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST_NOT      /StringBuilder::append/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 3/
//! INST          /StringBuilder::append/
//! INST_NOT      /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 2/
//! INST_NOT      /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 1/
//! INST_NOT      /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /StringBuilder::toString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::try_catch_concat_loop0"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,6
//! INST_COUNT    /StringBuilder::append/,8
//! INST_COUNT    /StringBuilder::toString/,6
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,6
//! INST_COUNT    /StringBuilder::append/,8
//! INST_COUNT    /StringBuilder::toString/,6
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::try_catch_concat_loop1"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,2
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::try_catch_concat_loop2"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,3
//! INST_COUNT    /StringBuilder::append/,3
//! INST_COUNT    /StringBuilder::toString/,3
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /StringBuilder::append/,3
//! INST_COUNT    /StringBuilder::toString/,2
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::try_catch_concat_loop3"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,2
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::try_catch_concat_loop4"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,1
//! INST_COUNT    /StringBuilder::toString/,1
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,1
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::try_catch_concat_loop5"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,5
//! INST_COUNT    /StringBuilder::append/,5
//! INST_COUNT    /StringBuilder::toString/,5
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,4
//! INST_COUNT    /StringBuilder::append/,5
//! INST_COUNT    /StringBuilder::toString/,5
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop0"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,4
//! INST_COUNT    /StringBuilder::append/,4
//! INST_COUNT    /StringBuilder::toString/,4
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,4
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop1"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,3
//! INST_COUNT    /StringBuilder::append/,3
//! INST_COUNT    /StringBuilder::toString/,3
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,3
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop2"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,3
//! INST_COUNT    /StringBuilder::append/,3
//! INST_COUNT    /StringBuilder::toString/,3
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,3
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop3"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,3
//! INST_COUNT    /StringBuilder::append/,3
//! INST_COUNT    /StringBuilder::toString/,3
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,3
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop4"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,3
//! INST_COUNT    /StringBuilder::append/,3
//! INST_COUNT    /StringBuilder::toString/,3
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,3
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop5"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop6"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop7"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop8"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop9"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop10"
//! PASS_AFTER    "BranchElimination"
//! IN_BLOCK      /loop 1, depth 1/,2
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,1
//! INST_COUNT    /StringBuilder::toString/,1
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,0
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 1, depth 1/,2
//! INST_COUNT    /StringBuilder::append/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop11"
//! PASS_AFTER    "BranchElimination"
//! IN_BLOCK      /loop 1, depth 1/,2
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,1
//! INST_COUNT    /StringBuilder::toString/,1
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,0
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 1, depth 1/,2
//! INST_COUNT    /StringBuilder::append/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::switch_concat_loop0"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,6
//! INST_COUNT    /StringBuilder::append/,6
//! INST_COUNT    /StringBuilder::toString/,6
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,6
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::switch_concat_loop1"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,6
//! INST_COUNT    /StringBuilder::append/,6
//! INST_COUNT    /StringBuilder::toString/,6
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,6
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuse_concat_loop0"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuse_concat_loop1"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuse_concat_loop2"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuse_concat_loop3"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuse_concat_loop4"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,2
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuse_concat_loop5"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,2
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::args_concat_loop0"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /StringBuilder::append/,4
//! INST_COUNT    /StringBuilder::toString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /StringBuilder::append/,4
//! INST_COUNT    /StringBuilder::toString/,2

//! CHECKER       INTERP-IRTOC, check String concatenation loop
//! SKIP_IF       @architecture == "arm32"
//! DEFINE_FRONTEND_OPTIONS            RUN_BCO reuse .abc
//! RUN           options: "--compiler-regex='.*concat_loop[0-9]*' --compiler-enable-jit=false --interpreter-type=irtoc --compiler-enable-events --compiler-events-path=./events.csv", entry: "ets_string_concat_loop.ETSGLOBAL::main"

//! CHECKER       INTERP-CPP, check String concatenation loop
//! SKIP_IF       @architecture == "arm32"
//! DEFINE_FRONTEND_OPTIONS            RUN_BCO reuse .abc
//! RUN           options: "--compiler-regex='.*concat_loop[0-9]*' --compiler-enable-jit=false --interpreter-type=cpp --compiler-enable-events --compiler-events-path=./events.csv", entry: "ets_string_concat_loop.ETSGLOBAL::main"

function concat_loop0(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i)
        str = str + a;                              // applied
    return str;
}

function concat_loop1(a: String, n: int): String {
    let str: String = "";
    while (--n >= 0)
        str += a;                                   // applied
    return str;
}

function concat_loop2(a: String, n: int): String {
    let str: String = "";
    do
        str += a;                                   // applied
    while (--n > 0);
    return str;
}

function concat_loop3(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i, str += a);          // applied
    return str;
}

function concat_loop4(strings: FixedArray<string>): String {
    let str: String = "";
    for (let i = 0; i < strings.length; ++i) {
        str += strings[i];                          // applied
    }
    return str;
}

function concat_loop5(strings: FixedArray<string>): String {
    let str: String = "";
    for (let i = 0; i < strings.length; ++i) {
        str += strings[0] + strings[i];             // applied
    }
    return str;
}

function concat_loop7(a: String, b: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += a;                                   // applied
        str += b;                                   // applied
    }
    return str;
}

function equivalent_concat_loop7(a: String, b: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        let sb0 = new StringBuilder();              // applied
        sb0.append(str);
        sb0.append(a);
        let tmp1 = sb0.toString();

        let sb1 = new StringBuilder();              // applied
        sb1.append(tmp1);
        sb1.append(b);
        str = sb1.toString();
    }
    return str;
}

function concat_loop8(a: String, b: String, n: int): String {
    let a1b: String = "";
    let b2a: String = "";
    for (let i = 0; i < n; ++i) {
        a1b += a + 1;                               // applied
        a1b += b;                                   // applied

        b2a += b + 2;                               // applied
        b2a += a;                                   // applied
    }
    return a1b + "." + b2a;
}

function nested_concat_loop0(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i)
        for (let j = 0; j < n; ++j)
            for (let k = 0; k < n; ++k)
                str = str + a;                      // applied, instructions moved out of outermost loop
    return str;
}

function try_catch_concat_loop0(a: String, n: int): String {
    let str_try: String = "";
    let str_catch: String = "";
    let str_finally: String = "";
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += a;                                   // not applied due to loop containing try-catch-finally blocks
                                                    // replaced by concat intrinsic
        try {
            str_try += a;                           // not applied due to being inside try-block
                                                    // replaced by concat intrinsic
        }
        catch(ex) {
            str_catch += a;                         // not applied due to being inside catch-block
                                                    // replaced by concat intrinsic
        }
        finally {
            str_finally += a;                       // not applied due to being inside finally-block
                                                    // replaced by concat intrinsic
        }
    }
    return str + str_try + str_catch + str_finally; // replaced by concat intrinsic
}

function try_catch_concat_loop1(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i)
        try {
            str = str + a;                          // not applied
                                                    // replaced by concat intrinsic
        }
        catch(ex) {
            str = str + a;                          // not applied
                                                    // replaced by concat intrinsic
        }
    return str;
}

function try_catch_concat_loop2(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i)
        try {
            str = str + a;                          // not applied
                                                    // replaced by concat intrinsic
        }
        finally {
            str = str + a;                          // partially applied
                                                    // SB merged with the one inside try-block
        }
    return str;
}

function try_catch_concat_loop3(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i)
        try {
            for (let j = 0; j < n; ++j) {
                str += a;                       // not applied due to loop containing try-catch-finally blocks
                                                // replaced by concat intrinsic
            }
        }
        catch(ex) {
            for (let j = 0; j < n; ++j) {
                str += a;                       // not applied
                                                // replaced by concat intrinsic
            }
        }
    return str;
}

function try_catch_concat_loop4(a: String, n: int): String {
    let str: String = "";
    try {
        for (let i = 0; i < n; ++i)
            str += a;                           // not applied due to loop containing try-catch-finally blocks
                                                // replaced by concat intrinsic
    }
    catch(ex) {}
    return str;
}

function try_catch_concat_loop5(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += a;                   // not applied due to loop containing try-catch-finally blocks
                                    // replaced by concat intrinsic
        try {
            str += a;               // partially applied
                                    // SBs inside try, catch, and finally blocks merged into one
        }
        catch(ex) {
            str += a;               // partially applied
                                    // SBs inside try, catch, and finally blocks merged into one
        }
        finally {
            str += a;               // partially applied
                                    // SBs inside try, catch, and finally blocks merged into one
        }
    }
    return str;
}

function if_concat_loop0(a: String, b: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += a;                               // applied
        if (i < n/2)
            str += 1;                           // applied
        else
            str += 2;                           // applied
        str += b;                               // applied
    }
    return str;
}

function if_concat_loop1(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += a;                               // applied
        if (i < n/2)
            str += 1;                           // applied
        else
            str += 2;                           // applied
    }
    return str;
}

function if_concat_loop2(a: String, b: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += a;                               // applied
        if (i < n/2)
            str += 1;                           // applied
        str += b;                               // applied
    }
    return str;
}

function if_concat_loop3(a: String, b: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += a;                               // applied
        if (i < n/2) {
        }
        else
            str += 2;                           // applied
        str += b;                               // applied
    }
    return str;
}

function if_concat_loop4(b: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        if (i < n/2)
            str += 1;                           // applied
        else
            str += 2;                           // applied
        str += b;                               // applied
    }
    return str;
}

function if_concat_loop5(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += a;                               // applied
        if (i < n/2)
            str += 1;                           // applied
    }
    return str;
}

function if_concat_loop6(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += a;                               // applied
        if (i < n/2) {
        }
        else
            str += 2;                           // applied
    }
    return str;
}

function if_concat_loop7(n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        if (i < n/2)
            str += 1;                           // applied
        else
            str += 2;                           // applied
    }
    return str;
}

function if_concat_loop8(b: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        if (i < n/2) {
        }
        else
            str += 2;                           // applied
        str += b;                               // applied
    }
    return str;
}

function if_concat_loop9(b: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        if (i < n/2)
            str += 1;                           // applied
        str += b;                               // applied
    }
    return str;
}

function if_concat_loop10(n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        if (i < n/2)
            str += 1;                           // applied
    }
    return str;
}

function if_concat_loop11(n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        if (i < n/2) {
        }
        else
            str += 2;                           // applied
    }
    return str;
}

function switch_concat_loop0(a: String, b: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += a;                               // applied
        switch (i) {
            case 0:
                str += 0;                       // applied
                break;
            case 1:
                str += 1;                       // applied
                break;
            case 2:
                str += 2;                       // applied
                break;
            default:
                str += i;                       // applied
                break;
        }
        str += b;                               // applied
    }
    return str;
}

function switch_concat_loop1(a: String, b: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += a;                               // applied
        switch (i) {
            case 0:
                str += 0;                       // applied
            case 1:
                str += 1;                       // applied
            case 2:
                str += 2;                       // applied
            default:
                str += i;                       // applied
        }
        str += b;                               // applied
    }
    return str;
}

function reuse_concat_loop0(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += str;                             // partially applied
        str += a;                               // two SBs merged into one
    }
    return str;
}

function reuse_concat_loop1(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += str.length;                      // partially applied
        str += a;                               // two SBs merged into one
    }
    return str;
}

function reuse_concat_loop2(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += str.getLength();                 // partially applied
        str += a;                               // two SBs merged into one
    }
    return str;
}

function reuse_concat_loop3(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += a;                               // not applied
        str += str;                             // unsupported string concatenation pattern
    }
    return str;
}

function reuse_concat_loop4(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += a;                               // partially applied
        str += str.length;                      // two SBs merged into one
    }
    return str;
}

function reuse_concat_loop5(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += a;                               // partially applied
        str += str.getLength();                 // two SBs merged into one
    }
    return str;
}

function args_concat_loop0(x: double, y: boolean, ...p: FixedArray<char>): string
{
    let res: string = "" + x + y + p.length;    // not replaced by intrinsic due to concatenating float, bool, int
    for (let ch of p) res += ch;                // not applied due to exit block not dominated by preheader
    return res;
}

function bugfix_concat_loop0(): String {
    let res = ""
    for (let i = 0; i < 1; ++i)
    {
        if (i < 0) {
            res += "0"
        }
        else
        {
            res += "12345"
            res += "678"
        }
    }
    return res
}

function bugfix_concat_loop1(size: int, charset: string): string {
    let res: string = new string()

    switch (charset) {
        case "ISO-8859-1":
            for (let i = 0; i < size; ++i)
                res += string.fromCharCode(0x43)
        break
        case "UTF-16":
            for (let i = 0; i < size; ++i)
                res += string.fromCharCode(0xD799)
        break
    }

    return res
}

function main() {
    const n: int = 2;
    arktest.assertEQ(concat_loop0("a", n), "aa", "Wrong result at concat_loop0");
    arktest.assertEQ(concat_loop1("a", n), "aa", "Wrong result at concat_loop1");
    arktest.assertEQ(concat_loop2("a", n), "aa", "Wrong result at concat_loop2");
    arktest.assertEQ(concat_loop3("a", n), "aa", "Wrong result at concat_loop3");
    arktest.assertEQ(concat_loop4(["a", "b", "c", "d"]), "abcd", "Wrong result at concat_loop4");
    arktest.assertEQ(concat_loop5(["a", "b", "c", "d"]), "aaabacad", "Wrong result at concat_loop5");
    arktest.assertEQ(concat_loop7("a", "b", n), "abab", "Wrong result at concat_loop7");
    arktest.assertEQ(equivalent_concat_loop7("a", "b", n), "abab", "Wrong result at equivalent_concat_loop7");
    arktest.assertEQ(concat_loop8("a", "b", n), "a1ba1b.b2ab2a", "Wrong result at concat_loop8");
    arktest.assertEQ(nested_concat_loop0("a", n), "aaaaaaaa", "Wrong result at nested_concat_loop0");
    arktest.assertEQ(try_catch_concat_loop0("a", n), "aaaaaa", "Wrong result at try_catch_concat_loop0");
    arktest.assertEQ(try_catch_concat_loop1("a", n), "aa", "Wrong result at try_catch_concat_loop1");
    arktest.assertEQ(try_catch_concat_loop2("a", n), "aaaa", "Wrong result at try_catch_concat_loop2");
    arktest.assertEQ(try_catch_concat_loop3("a", n), "aaaa", "Wrong result at try_catch_concat_loop3");
    arktest.assertEQ(try_catch_concat_loop4("a", n), "aa", "Wrong result at try_catch_concat_loop4");
    arktest.assertEQ(try_catch_concat_loop5("a", n), "aaaaaa", "Wrong result at try_catch_concat_loop5");
    arktest.assertEQ(if_concat_loop0("a", "b", n), "a1ba2b", "Wrong result at if_concat_loop0");
    arktest.assertEQ(if_concat_loop1("a", n), "a1a2", "Wrong result at if_concat_loop1");
    arktest.assertEQ(if_concat_loop2("a", "b", n), "a1bab", "Wrong result at if_concat_loop2");
    arktest.assertEQ(if_concat_loop3("a", "b", n), "aba2b", "Wrong result at if_concat_loop3");
    arktest.assertEQ(if_concat_loop4("b", n), "1b2b", "Wrong result at if_concat_loop4");
    arktest.assertEQ(if_concat_loop5("a", n), "a1a", "Wrong result at if_concat_loop5");
    arktest.assertEQ(if_concat_loop6("a", n), "aa2", "Wrong result at if_concat_loop6");
    arktest.assertEQ(if_concat_loop7(n), "12", "Wrong result at if_concat_loop7");
    arktest.assertEQ(if_concat_loop8("b", n), "b2b", "Wrong result at if_concat_loop8");
    arktest.assertEQ(if_concat_loop9("b", n), "1bb", "Wrong result at if_concat_loop9");
    arktest.assertEQ(if_concat_loop10(n), "1", "Wrong result at if_concat_loop10");
    arktest.assertEQ(if_concat_loop11(n), "2", "Wrong result at if_concat_loop11");
    arktest.assertEQ(switch_concat_loop0("a", "b", n), "a0ba1b", "Wrong result at switch_concat_loop0");
    arktest.assertEQ(switch_concat_loop1("a", "b", n), "a0120ba121b", "Wrong result at switch_concat_loop1");
    arktest.assertEQ(reuse_concat_loop0("a", n), "aaa", "Wrong result at reuse_concat_loop0");
    arktest.assertEQ(reuse_concat_loop1("a", n), "0a2a", "Wrong result at reuse_concat_loop1");
    arktest.assertEQ(reuse_concat_loop2("a", n), "0a2a", "Wrong result at reuse_concat_loop2");
    arktest.assertEQ(reuse_concat_loop3("a", n), "aaaaaa", "Wrong result at reuse_concat_loop3");
    arktest.assertEQ(reuse_concat_loop4("a", n), "a1a3", "Wrong result at reuse_concat_loop4");
    arktest.assertEQ(reuse_concat_loop5("a", n), "a1a3", "Wrong result at reuse_concat_loop5");
    arktest.assertEQ(args_concat_loop0(1.5, false, c'A', c'B'), "1.5false2AB", "Wrong result at args_concat_loop0");
    arktest.assertEQ(bugfix_concat_loop0(), "12345678", "Wrong result at bugfix_concat_loop0");
    arktest.assertEQ(bugfix_concat_loop1(n, "ISO-8859-1"), "CC", "Wrong result at bugfix_concat_loop1");
    arktest.assertEQ(bugfix_concat_loop1(n, "UTF-16"), "힙힙", "Wrong result at bugfix_concat_loop1");
}
