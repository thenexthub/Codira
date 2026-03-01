/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

//! CHECKER      Try Catch Resolving JIT Optimize when default catch is outside
//! RUN          force_jit: true, options: "--compiler-hotness-threshold=20 --compiler-profiling-threshold=0 --compiler-enable-osr=false --compiler-regex=ets_try_catch.MultipleCatch::__noinline__multipleCatch", entry: "ets_try_catch.ETSGLOBAL::main"
//! EVENT        /Compilation,ets_try_catch.MultipleCatch::__noinline__multipleCatch,.*,COMPILED/
//! METHOD       "ets_try_catch.MultipleCatch::__noinline__multipleCatch"
//! PASS_AFTER   "Inline"
//! INST_COUNT   "Throw",2
//! PASS_AFTER   "TryCatchResolving"
//! INST_NOT     "Throw"

//! CHECKER      Try Catch Resolving AOT Optimize when default catch is outside
//! SKIP_IF      @architecture == "arm32"
//! RUN_PAOC     options: "--compiler-regex=ets_try_catch.MultipleCatch::__noinline__multipleCatch"
//! METHOD       "ets_try_catch.MultipleCatch::__noinline__multipleCatch"
//! PASS_AFTER   "Inline"
//! INST_COUNT   "Throw",2
//! PASS_AFTER   "TryCatchResolving"
//! INST_COUNT   "Throw",2

//! CHECKER      Try Catch Resolving AOT+PGO Optimize when default catch is outside
//! SKIP_IF      @architecture == "arm32"
//! RUN_PGO_PROF entry: "ets_try_catch.ETSGLOBAL::main"
//! RUN_PGO_PAOC options: "--compiler-regex=ets_try_catch.MultipleCatch::__noinline__multipleCatch"
//! METHOD       "ets_try_catch.MultipleCatch::__noinline__multipleCatch"
//! PASS_AFTER   "Inline"
//! INST_COUNT   "Throw",2
//! PASS_AFTER   "TryCatchResolving"
//! INST_NOT     "Throw"

class Error0 extends Error {}
class Error1 extends Error {}

class MultipleCatch {
  __noinline__defaultCatch() : int {
     let res : int = 0;
     try {
         res = this.__noinline__multipleCatch();
     } catch(e) {
     }
     return res;
  }

  __noinline__multipleCatch() : int {
    let res : int = 0;
    for (let i : int = 0; i < 16; i++) {
      let k : int = i % 2;
      try {
        switch (k) {
          case 0:
            throw new Error0();
          case 1:
            throw new Error1();
          default:
            break;
        }
      } catch (e) {
        if (e instanceof Error0) {
          res += 1;
        } else if (e instanceof Error1) {
          res += 2;
        }
      }
    }
    return res;
  }
}

let mc = new MultipleCatch();

function main(): int {
    let res :int = 0;
    for (let i:int = 0; i < 10; ++i) {
        res += mc.__noinline__defaultCatch();
    }
    return res - 240;
}
