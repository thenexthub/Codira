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

//! CHECKER      Check for typeof + compare peephole
//! RUN          force_jit: true, options: "--compiler-hotness-threshold=20 --compiler-profiling-threshold=0 --compiler-enable-osr=false", entry: "ets_typeof_compare3.ETSGLOBAL::main"
//! EVENT        /Compilation,ets_typeof_compare3.ETSGLOBAL::__noinline__process_typeof,.*,COMPILED/
//! METHOD       "ets_typeof_compare3.ETSGLOBAL::__noinline__process_typeof"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   "Intrinsic.StdCoreRuntimeTypeof",1
//! INST_NOT     "IsInstance"
//! PASS_BEFORE  "LocationsBuilder"
//! INST_NOT     "Intrinsic.StdCoreRuntimeTypeof"
//! INST_COUNT   "IsInstance",1

function __noinline__process_typeof(u: string|short): int {
   if (typeof (u) == "number") {
     return 1;
   } else {
     return 2;
   }
}

function main(): int {
  let res :int = 0;
  for (let i:short = 0; i < 25; ++i) {
    res += __noinline__process_typeof(i);
  }
  console.log(res);
  return res - 25;
}
