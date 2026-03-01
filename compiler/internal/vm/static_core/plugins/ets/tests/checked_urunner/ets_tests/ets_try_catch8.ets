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

//! CHECKER      Try Catch Resolving JIT optimize with nested catches
//! RUN          force_jit: true, options: "--compiler-regex='.*(multipleCatch|wrapper|main)' --compiler-hotness-threshold=5 --compiler-profiling-threshold=0 --compiler-inlining-max-insts=160 --compiler-enable-osr=false", entry: "ets_try_catch8.ETSGLOBAL::main"
//! EVENT        /Compilation,ets_try_catch8.MultipleCatch::multipleCatch,.*,COMPILED/
//! EVENT        /Compilation,ets_try_catch8.MultipleCatch::wrapper,.*,COMPILED/
//! EVENT        /Inline,ets_try_catch8.ETSGLOBAL::main,ets_try_catch8.MultipleCatch::wrapper,.*,SUCCESS/
//! EVENT_NOT    /Inline,ets_try_catch8.MultipleCatch::wrapper,ets_try_catch8.MultipleCatch::multipleCatch,.*,SUCCESS/
//! METHOD       "ets_try_catch8.MultipleCatch::multipleCatch"
//! PASS_AFTER   "Inline"
//! INST_COUNT   "Throw",2
//! PASS_AFTER   "TryCatchResolving"
//! INST_NOT     "Throw"

//! CHECKER      Try Catch Resolving AOT optimize with nested catches
//! SKIP_IF      @architecture == "arm32"
//! RUN_PAOC     options: "--compiler-regex=ets_try_catch8.MultipleCatch::multipleCatch"
//! METHOD       "ets_try_catch8.MultipleCatch::multipleCatch"
//! PASS_AFTER   "Inline"
//! INST_COUNT   "Throw",2
//! PASS_AFTER   "TryCatchResolving"
//! INST_COUNT   "Throw",2

//! CHECKER      Try Catch Resolving AOT+PGO optimize with nested catches
//! SKIP_IF      @architecture == "arm32"
//! RUN_PGO_PROF entry: "ets_try_catch8.ETSGLOBAL::main"
//! RUN_PGO_PAOC options: "--compiler-regex=ets_try_catch8.MultipleCatch::multipleCatch"
//! METHOD       "ets_try_catch8.MultipleCatch::multipleCatch"
//! PASS_AFTER   "Inline"
//! INST_COUNT   "Throw",2
//! PASS_AFTER   "TryCatchResolving"
//! INST_NOT     "Throw"

class Error0 extends Error {}

class MultipleCatch {
    multipleCatch(a: int, b: int, c: int) : int {
    let res : int = 0;

    for (let i : int = 0; i < 16; i++) {
      let k : int = i % 2;
      let a1 : int = 0;
      let b1 : int = 0;
      let c1 : int = 0;
      try {
        switch (k) {
          case 0:
            a1 = a + i;
            b1 = b + i;
            c1 = c >> i;
            throw new Error0();
          case 1:
            a1 = a - i;
            b1 = b - i;
            c1 = c << i;
            throw new Error0();
          default:
            break;
        }
      } catch (e) {
        if (e instanceof Error0) {
          res += a1 + b1 + c1;
        }
      }
    }
    return res;
  }
  wrapper(a :int, b :int, c :int, d :int): int {
    return d + this.multipleCatch(a, b, c);
  }
}

let mc = new MultipleCatch();

function main(): int {
    console.println("in main")
    let res :int = 0;
    let a :int = 3;
    let b :int = 2;
    let c :int = 1;
    let d :int = 100;
    for (let i :int = 0; i < 10; ++i) {
        try {
            res += mc.wrapper(a, b, c, d);
        } catch (e) {
            ;
        }
    }
    return res - 438550;
}
