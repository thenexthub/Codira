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

//! CHECKER     Concat loop with length access unoptimized loop AOT
//! SKIP_IF     @architecture == "arm32"
//! RUN_PAOC    options: "--compiler-regex='.*reuse_concat_loop1' --compiler-optimize-string-concat=true --compiler-inlining=false --compiler-check-final=false --compiler-simplify-string-builder=false"
//! RUN         options: "--compiler-regex='.*reuse_concat_loop1'",  entry: "ets_stringbuilder_length_part3.ETSGLOBAL::main", result: 0

//! CHECKER     Concat loop with length access loop AOT CHA inline
//! SKIP_IF     @architecture == "arm32"
//! RUN_PAOC    options: "--compiler-regex='.*reuse_concat_loop1' --compiler-optimize-string-concat=true --compiler-inlining=true --paoc-use-cha=true --compiler-check-final=false"
//!
//! METHOD      "ets_stringbuilder_length_part3.ETSGLOBAL::reuse_concat_loop1"
//! PASS_BEFORE "SimplifyStringBuilder"
//! INST_COUNT  /StringBuilder::<ctor>/,1
//! INST_COUNT  /Intrinsic.StdCoreSbToString/,1
//! INST_COUNT  /LenArray/,0
//! INST_COUNT  /LoadObject \d+ std.core.StringBuilder.length/,0
//! PASS_AFTER  "SimplifyStringBuilder"
//! INST_COUNT  /StringBuilder::<ctor>/,1
//! INST_COUNT  /Intrinsic.StdCoreSbToString/,1
//! INST_NOT    /LenArray/
//! INST_COUNT  /LoadObject \d+ std.core.StringBuilder.length/,0
//!
//! RUN         options: "--compiler-regex='.*reuse_concat_loop1'", entry: "ets_stringbuilder_length_part3.ETSGLOBAL::main", result: 0

//! CHECKER     BCO, StringBuilder getStringLength calls optimized
//! RUN_BCO     options: "--ets-strings-concat=false --bco-compiler --compiler-simplify-string-builder=true --bco-compiler --compiler-optimize-string-concat=true"
//!
//! METHOD      "ets_stringbuilder_length_part3.ETSGLOBAL::getStringLengthCount31"
//! PASS_AFTER  "BranchElimination"
//! INST_COUNT  /std.core.StringBuilder::%%get-stringLength/,3
//! PASS_AFTER  "SimplifyStringBuilder"
//! INST_COUNT  /std.core.StringBuilder::%%get-stringLength/,1
//!
//! METHOD      "ets_stringbuilder_length_part3.ETSGLOBAL::getStringLengthCount32"
//! PASS_AFTER  "BranchElimination"
//! INST_COUNT  /std.core.StringBuilder::%%get-stringLength/,3
//! PASS_AFTER  "SimplifyStringBuilder"
//! INST_COUNT  /std.core.StringBuilder::%%get-stringLength/,2
//!
//! METHOD      "ets_stringbuilder_length_part3.ETSGLOBAL::getStringLengthCount22"
//! PASS_AFTER  "BranchElimination"
//! INST_COUNT  /std.core.StringBuilder::%%get-stringLength/,2
//! PASS_AFTER  "SimplifyStringBuilder"
//! INST_COUNT  /std.core.StringBuilder::%%get-stringLength/,2
//!
//! METHOD      "ets_stringbuilder_length_part3.ETSGLOBAL::getStringLengthCountUpdated"
//! PASS_AFTER  "BranchElimination"
//! INST_COUNT  /std.core.StringBuilder::%%get-stringLength/,2
//! PASS_AFTER  "SimplifyStringBuilder"
//! INST_COUNT  /std.core.StringBuilder::%%get-stringLength/,2
//!
//! METHOD      "ets_stringbuilder_length_part3.ETSGLOBAL::getStringLengthCountNewSb"
//! PASS_AFTER  "BranchElimination"
//! INST_COUNT  /std.core.StringBuilder::%%get-stringLength/,2
//! PASS_AFTER  "SimplifyStringBuilder"
//! INST_COUNT  /std.core.StringBuilder::%%get-stringLength/,2
//! RUN         entry: "ets_stringbuilder_length_part3.ETSGLOBAL::main", result: 0

//! DEFINE_FRONTEND_OPTIONS            BCO compiler-simplify-string-builder=false
//! RUN_FRONTEND            options: "--ets-strings-concat=false --bco-compiler --compiler-simplify-string-builder=false"

//! CHECKER      AOT, StringBuilder getStringLength calls optimized
//! SKIP_IF      @architecture == "arm32"
//! DEFINE_FRONTEND_OPTIONS            BCO compiler-simplify-string-builder=false
//! RUN_PAOC     options: "--compiler-inlining=false --compiler-optimize-string-concat=false  --compiler-simplify-string-builder=true --compiler-check-final=false"
//!
//! METHOD      "ets_stringbuilder_length_part3.ETSGLOBAL::getStringLengthCount31"
//! PASS_AFTER  "BranchElimination"
//! INST_COUNT  /std.core.StringBuilder::%%get-stringLength/,3
//! PASS_AFTER  "SimplifyStringBuilder"
//! INST_COUNT  /std.core.StringBuilder::%%get-stringLength/,1
//!
//! METHOD      "ets_stringbuilder_length_part3.ETSGLOBAL::getStringLengthCount32"
//! PASS_AFTER  "BranchElimination"
//! INST_COUNT  /std.core.StringBuilder::%%get-stringLength/,3
//! PASS_AFTER  "SimplifyStringBuilder"
//! INST_COUNT  /std.core.StringBuilder::%%get-stringLength/,2
//!
//! METHOD      "ets_stringbuilder_length_part3.ETSGLOBAL::getStringLengthCount22"
//! PASS_AFTER  "BranchElimination"
//! INST_COUNT  /std.core.StringBuilder::%%get-stringLength/,2
//! PASS_AFTER  "SimplifyStringBuilder"
//! INST_COUNT  /std.core.StringBuilder::%%get-stringLength/,2
//!
//! METHOD      "ets_stringbuilder_length_part3.ETSGLOBAL::getStringLengthCountUpdated"
//! PASS_AFTER  "BranchElimination"
//! INST_COUNT  /std.core.StringBuilder::%%get-stringLength/,2
//! PASS_AFTER  "SimplifyStringBuilder"
//! INST_COUNT  /std.core.StringBuilder::%%get-stringLength/,2
//!
//! METHOD      "ets_stringbuilder_length_part3.ETSGLOBAL::getStringLengthCountNewSb"
//! PASS_AFTER  "BranchElimination"
//! INST_COUNT  /std.core.StringBuilder::%%get-stringLength/,2
//! PASS_AFTER  "SimplifyStringBuilder"
//! INST_COUNT  /std.core.StringBuilder::%%get-stringLength/,2
//!
//! RUN          options: "", entry: "ets_stringbuilder_length_part3.ETSGLOBAL::main", result: 0

//! CHECKER      JIT, StringBuilder getStringLength calls BCO optimized
//! SKIP_IF      @architecture == "arm32"
//! DEFINE_FRONTEND_OPTIONS            BCO compiler-simplify-string-builder=false
//! RUN          force_jit: true, options: "", entry: "ets_stringbuilder_length_part3.ETSGLOBAL::main", result: 0
//!
//! METHOD      "ets_stringbuilder_length_part3.ETSGLOBAL::getStringLengthCount31"
//! PASS_AFTER  "BranchElimination"
//! INST_COUNT  /std.core.StringBuilder::%%get-stringLength/,3
//! PASS_AFTER  "SimplifyStringBuilder"
//! INST_COUNT  /std.core.StringBuilder::%%get-stringLength/,1
//!
//! METHOD      "ets_stringbuilder_length_part3.ETSGLOBAL::getStringLengthCount32"
//! PASS_AFTER  "BranchElimination"
//! INST_COUNT  /std.core.StringBuilder::%%get-stringLength/,3
//! PASS_AFTER  "SimplifyStringBuilder"
//! INST_COUNT  /std.core.StringBuilder::%%get-stringLength/,2
//!
//! METHOD      "ets_stringbuilder_length_part3.ETSGLOBAL::getStringLengthCount22"
//! PASS_AFTER  "BranchElimination"
//! INST_COUNT  /std.core.StringBuilder::%%get-stringLength/,2
//! PASS_AFTER  "SimplifyStringBuilder"
//! INST_COUNT  /std.core.StringBuilder::%%get-stringLength/,2
//!
//! METHOD      "ets_stringbuilder_length_part3.ETSGLOBAL::getStringLengthCountUpdated"
//! PASS_AFTER  "BranchElimination"
//! INST_COUNT  /std.core.StringBuilder::%%get-stringLength/,2
//! PASS_AFTER  "SimplifyStringBuilder"
//! INST_COUNT  /std.core.StringBuilder::%%get-stringLength/,2
//!
//! METHOD      "ets_stringbuilder_length_part3.ETSGLOBAL::getStringLengthCountNewSb"
//! PASS_AFTER  "BranchElimination"
//! INST_COUNT  /std.core.StringBuilder::%%get-stringLength/,2
//! PASS_AFTER  "SimplifyStringBuilder"
//! INST_COUNT  /std.core.StringBuilder::%%get-stringLength/,2

function reuse_concat_loop1(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += str.length;                      // partially applied
        str += a;                               // two SBs merged into one
    }    
    return str;
}

function getStringLengthCount31(){
  let sb = new StringBuilder();
  let len0 = sb.stringLength;
  let len1 = sb.stringLength;
  let len2 = sb.stringLength;
  return len2;
}

function getStringLengthCount32(){
  let sb = new StringBuilder();
  let len0 = sb.stringLength;
  let len1 = sb.stringLength;
  sb.append(0)
  let len2 = sb.stringLength;
  return len2
}

function getStringLengthCount22(){
  let sb = new StringBuilder();
  let len0 = sb.stringLength;
  if (true)
    sb.append(0);
  let len1 = sb.stringLength;
  return len1;
}

function StringUpdate(sb: StringBuilder): void {    
    sb.append(".");
}

function getStringLengthCountUpdated(){
  let sb = new StringBuilder();
  let len0 = sb.stringLength;
  StringUpdate(sb);
  let len1 = sb.stringLength;
  return len1;
}

function getStringLengthCountNewSb(){
  let sb = new StringBuilder();
  let len0 = sb.stringLength;
  let newSb = new StringBuilder();
  let len1 = newSb.stringLength;
  return len1;
}

function main() {
    arktest.assertEQ(reuse_concat_loop1("a", 4), "0a2a4a6a", "Wrong result at reuse_concat_loop1");
    arktest.assertEQ(getStringLengthCount31(), 0, "Wrong result at getStringLengthCount31");
    arktest.assertEQ(getStringLengthCount32(), 1, "Wrong result at getStringLengthCount32");
    arktest.assertEQ(getStringLengthCount22(), 1, "Wrong result at getStringLengthCount22");
    arktest.assertEQ(getStringLengthCountUpdated(), 1, "Wrong result at getStringLengthCountUpdated");
    arktest.assertEQ(getStringLengthCountNewSb(), 0, "Wrong result at getStringLengthCountNewSb");
}

