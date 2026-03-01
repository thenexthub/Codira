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

//! CHECKER      Check for safepoints without placement
//! RUN          force_jit: true, options: "--compiler-regex=ets_safepoints.ETSGLOBAL::__noinline__func --compiler-enforce-safepoint-placement=false --compiler-hotness-threshold=20 --compiler-profiling-threshold=0 --compiler-enable-osr=false", entry: "ets_safepoints.ETSGLOBAL::main"
//! EVENT        /Compilation,ets_safepoints.ETSGLOBAL::__noinline__func,.*,COMPILED/
//! METHOD       "ets_safepoints.ETSGLOBAL::__noinline__func"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   "SafePoint",1

//! CHECKER      Check for safepoints with placement
//! RUN          force_jit: true, options: "--compiler-regex=ets_safepoints.ETSGLOBAL::__noinline__func --compiler-enforce-safepoint-placement=true --compiler-hotness-threshold=20 --compiler-profiling-threshold=0 --compiler-enable-osr=false", entry: "ets_safepoints.ETSGLOBAL::main"
//! EVENT        /Compilation,ets_safepoints.ETSGLOBAL::__noinline__func,.*,COMPILED/
//! METHOD       "ets_safepoints.ETSGLOBAL::__noinline__func"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   "SafePoint",4

//! CHECKER      Check for safepoints with placement for distance 0
//! RUN          force_jit: true, options: "--compiler-regex=ets_safepoints.ETSGLOBAL::__noinline__func --compiler-enforce-safepoint-placement=true --compiler-safepoint-distance-limit=0 --compiler-hotness-threshold=20 --compiler-profiling-threshold=0 --compiler-enable-osr=false", entry: "ets_safepoints.ETSGLOBAL::main"
//! EVENT        /Compilation,ets_safepoints.ETSGLOBAL::__noinline__func,.*,COMPILED/
//! METHOD       "ets_safepoints.ETSGLOBAL::__noinline__func"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   "SafePoint",6

//! CHECKER      Check for safepoints with placement for distance 1
//! RUN          force_jit: true, options: "--compiler-regex=ets_safepoints.ETSGLOBAL::__noinline__func --compiler-enforce-safepoint-placement=true --compiler-safepoint-distance-limit=1 --compiler-hotness-threshold=20 --compiler-profiling-threshold=0 --compiler-enable-osr=false", entry: "ets_safepoints.ETSGLOBAL::main"
//! EVENT        /Compilation,ets_safepoints.ETSGLOBAL::__noinline__func,.*,COMPILED/
//! METHOD       "ets_safepoints.ETSGLOBAL::__noinline__func"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   "SafePoint",6

function __noinline__func(k: int): int {
   let l :int = 6;
   if (k > 15) {
     k -= l;
     return k;
   } else {
     k += l; 
     return k;
   }
}

function main(): int {
  let res :int = 0;
  for (let i:int = 0; i < 25; ++i) {
    res += __noinline__func(i + 10);
  }
  console.log(res);
    return res - 728;
}
