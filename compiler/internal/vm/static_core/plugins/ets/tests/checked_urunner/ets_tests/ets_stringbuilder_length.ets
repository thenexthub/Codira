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
//! RUN_FRONTEND            options: "--ets-strings-concat=false --bco-compiler --compiler-simplify-string-builder=false"

//! CHECKER     StringBuilder length field access AOT unoptimized
//! SKIP_IF     @architecture == "arm32"
//! RUN_PAOC    options: "--compiler-regex='.*stringCalculation0[0-1]' --compiler-optimize-string-concat=true --compiler-inlining=false --compiler-check-final=false --compiler-simplify-string-builder=false"
//!
//! METHOD      "ets_stringbuilder_length.ETSGLOBAL::stringCalculation00"
//! PASS_AFTER  "OptimizeStringConcat"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT  /StringBuilder::<ctor>/,3
//! INST_COUNT  /Intrinsic.StdCoreSbToString/,3
//! INST_COUNT  /LenArray/,4
//! INST_NOT    /LoadObject \d+ std.core.StringBuilder.length/
//!
//! RUN         entry: "ets_stringbuilder_length.ETSGLOBAL::main", result: 0

//! CHECKER     StringBuilder length field access AOT optimized
//! SKIP_IF     @architecture == "arm32"
//! RUN_PAOC    options: "--compiler-regex='.*stringCalculation0[0-1]' --compiler-optimize-string-concat=true --compiler-inlining=false --compiler-check-final=false"
//!
//! METHOD      "ets_stringbuilder_length.ETSGLOBAL::stringCalculation00"
//! PASS_AFTER  "OptimizeStringConcat"
//! INST_COUNT  /StringBuilder::<ctor>/,3
//! INST_COUNT  /Intrinsic.StdCoreSbToString/,3
//! INST_COUNT  /LenArray/,4
//! INST_NOT    /LoadObject \d+ std.core.StringBuilder.length/
//! PASS_AFTER  "SimplifyStringBuilder"
//! INST_COUNT  /StringBuilder::<ctor>/,1
//! INST_COUNT  /Intrinsic.StdCoreSbToString/,1
//! INST_COUNT  /LenArray/,2
//! INST        /LoadObject \d+ std.core.StringBuilder.length/
//!
//! METHOD      "ets_stringbuilder_length.ETSGLOBAL::stringCalculation01"
//! PASS_AFTER  "OptimizeStringConcat"
//! INST_COUNT  /StringBuilder::<ctor>/,3
//! INST_COUNT  /Intrinsic.StdCoreSbToString/,3
//! PASS_AFTER  "SimplifyStringBuilder"
//! INST_COUNT  /StringBuilder::<ctor>/,1
//! INST_COUNT  /Intrinsic.StdCoreSbToString/,1
//!
//! RUN         entry: "ets_stringbuilder_length.ETSGLOBAL::main", result: 0

//! DEFINE_FRONTEND_OPTIONS            RUN_BCO reuse .abc
//! RUN_BCO     options: "--ets-strings-concat=false --bco-compiler --compiler-simplify-string-builder=true --bco-compiler --compiler-optimize-string-concat=true"

//! CHECKER     BCO, StringBuilder length field access BCO optimized
//! DEFINE_FRONTEND_OPTIONS            RUN_BCO reuse .abc
//!
//! METHOD      "ets_stringbuilder_length.ETSGLOBAL::stringCalculation00"
//! PASS_AFTER  "BranchElimination"
//! INST_COUNT  /StringBuilder::<ctor>/,3
//! INST_COUNT  /StringBuilder::append/,3
//! INST_COUNT  /StringBuilder::toString/,3
//! INST_COUNT  /std.core.String::%%get-length/,4
//! INST_NOT    /std.core.StringBuilder::%%get-stringLength/
//! PASS_AFTER  "SimplifyStringBuilder"
//! INST_COUNT  /StringBuilder::<ctor>/,1
//! INST_COUNT  /StringBuilder::append/,3
//! INST_COUNT  /StringBuilder::toString/,1
//! INST_COUNT  /std.core.String::%%get-length/,2
//! INST_COUNT  /std.core.StringBuilder::%%get-stringLength/,1
//! IN_BLOCK    /loop 1, depth 1, bc: 0x000000a3/
//! INST_NOT    /StringBuilder::<ctor>/
//! INST_COUNT  /StringBuilder::append/,2
//! INST_NOT    /StringBuilder::toString/
//! IN_BLOCK    /bc:/
//! INST_NOT    /StringBuilder::<ctor>/
//! INST_NOT    /StringBuilder::append/
//! INST_COUNT  /StringBuilder::toString/,1
//! INST_COUNT  /std.core.String::%%get-length/,2

//! CHECKER       INTERP-IRTOC, StringBuilder length field access BCO optimized
//! SKIP_IF       @architecture == "arm32"
//! DEFINE_FRONTEND_OPTIONS            RUN_BCO reuse .abc
//! RUN           options: "--compiler-regex='.*stringCalculation0[0-1]' --compiler-enable-jit=false --interpreter-type=irtoc --compiler-enable-events --compiler-events-path=./events.csv", entry: "ets_stringbuilder_length.ETSGLOBAL::main", result: 0

//! CHECKER       INTERP-CPP, StringBuilder length field access BCO optimized
//! SKIP_IF       @architecture == "arm32"
//! DEFINE_FRONTEND_OPTIONS            RUN_BCO reuse .abc
//! RUN           options: "--compiler-regex='.*stringCalculation0[0-1]' --compiler-enable-jit=false --interpreter-type=cpp --compiler-enable-events --compiler-events-path=./events.csv", entry: "ets_stringbuilder_length.ETSGLOBAL::main", result: 0

function generateFakeRandomInteger(): Int32Array {
  let resource: Int32Array = new Int32Array([12, 43, 56, 76, 89, 54, 45, 32, 35, 47, 46, 44, 21, 37, 84]);
  return resource;
}

const N: int = 8;

function stringCalculation00() {
  let str1: string = 'h';
  let res: string = '11';

  let resources: Int32Array = generateFakeRandomInteger();
  for (let i: int = 0; i < N; i++) {
    if (resources[i % res.length.toInt() % 15].toInt() > resources[(i + resources[i % 15].toInt()) % res.length.toInt() % 15].toInt()) {
      res += str1 + i;
    } else {
      res += str1;
    }
  }
  console.log(res.length)
  return res.length.toInt()
}

function stringCalculation01() {
  let str1: string = 'h';
  let res: string = '11';

  let resources: Int32Array = generateFakeRandomInteger();
  for (let i: int = 0; i < N; i++) {
    let j = (i + resources[i % 15].toInt()) % 15;
    if (resources[i % 15].toInt() > resources[j].toInt()) {
      res += str1 + i;
    } else {
      res += str1;
    }
  }

  return res.length
}


function main() {
  arktest.assertEQ(stringCalculation00(), 11, 'Wrong result at stringCalculation00');
  arktest.assertEQ(stringCalculation01(), 12, 'Wrong result at stringCalculation01');
}
