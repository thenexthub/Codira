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

//! CHECKER     StringBuilder length field access AOT optimized
//! SKIP_IF     @architecture == "arm32"
//! RUN_PAOC    options: "--compiler-regex='.*StringCalculation02' --compiler-optimize-string-concat=true --compiler-inlining=false --compiler-check-final=false", entry: "ets_stringbuilder_length_part2.ETSGLOBAL::main", result: 0
//!
//! METHOD      "ets_stringbuilder_length_part2.ETSGLOBAL::StringCalculation02"
//! PASS_AFTER  "OptimizeStringConcat"
//! INST_COUNT  /StringBuilder::<ctor>/,2
//! INST_COUNT  /Intrinsic.StdCoreSbToString/,2
//! INST_COUNT  /LenArray/,1
//! INST_NOT    /LoadObject \d+ std.core.StringBuilder.length/
//! PASS_AFTER  "SimplifyStringBuilder"
//! INST_COUNT  /StringBuilder::<ctor>/,1
//! INST_COUNT  /Intrinsic.StdCoreSbToString/,1
//! INST_COUNT  /LenArray/,1
//! INST_NOT    /LoadObject \d+ std.core.StringBuilder.length/
//!
//! RUN         options: "--compiler-regex='.*StringCalculation02'", entry: "ets_stringbuilder_length_part2.ETSGLOBAL::main", result: 0

//! CHECKER     INTERP-CPP, StringBuilder length field access
//! RUN         options: "--compiler-enable-jit=false  --interpreter-type=cpp", entry: "ets_stringbuilder_length_part2.ETSGLOBAL::main"

function GenerateFakeRandomInteger(): Int32Array {
  let resource: Int32Array = new Int32Array([12, 43, 56, 76, 89, 54, 45, 32, 35, 47, 46, 44, 21, 37, 84]);
  return resource;
}

function StringCalculation02() {
    let outterCount: int = 2;
    let innerCount: int = 4;
    let str1: string = "h";
    let str2: string = "j";
    let res: string = "11";
  
    let resources: Int32Array = GenerateFakeRandomInteger();
    for (let i: int = 0; i < outterCount; i++) {
        res += str2 + i;
        for (let j: int = 0; j < innerCount; j++) {
          if (resources[j % res.length.toInt() % 15].toInt() > resources[(j + resources[i % 15].toInt()) % res.length.toInt() % 15].toInt()) {
            res += str1 + j;
          } else {
            res += str1;
          }
        }
    }
  
    return res.length.toInt() - 17
}

function main() {
  arktest.assertEQ(StringCalculation02(), 0, "Wrong result at StringCalculation02");
}
