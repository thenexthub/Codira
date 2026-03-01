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

//! CHECKER      Try Catch Resolving JIT Do not optimize inlined throws
//! RUN          force_jit: true, options: "--compiler-regex='.*MultipleCatch::__noinline__multipleCatch' --compiler-hotness-threshold=20 --compiler-profiling-threshold=0 --compiler-enable-osr=false --compiler-inlining-max-insts=160", entry: "ets_try_catch4.ETSGLOBAL::main"
//! EVENT        /Compilation,ets_try_catch4.MultipleCatch::__noinline__multipleCatch,.*,COMPILED/
//! EVENT        /Inline,ets_try_catch4.MultipleCatch::__noinline__multipleCatch,ets_try_catch4.ETSGLOBAL::throw_errors,.*,STATIC,SUCCESS/
//! METHOD       "ets_try_catch4.MultipleCatch::__noinline__multipleCatch"
//! PASS_AFTER   "Inline"
//! INST_COUNT   "Throw",2
//! PASS_AFTER   "TryCatchResolving"
//! INST_COUNT   "Throw",2

//! CHECKER      Try Catch Resolving AOT with inlined throws
//! SKIP_IF      @architecture == "arm32"
//! RUN_PAOC     options: "--compiler-regex=ets_try_catch4.MultipleCatch::__noinline__multipleCatch"
//! METHOD       "ets_try_catch4.MultipleCatch::__noinline__multipleCatch"
//! PASS_AFTER   "Inline"
//! INST_COUNT   "Throw",2
//! PASS_AFTER   "TryCatchResolving"
//! INST_COUNT   "Throw",2

//! CHECKER      Try Catch Resolving AOT+PGO with inlined throws
//! SKIP_IF      @architecture == "arm32"
//! RUN_PGO_PROF entry: "ets_try_catch4.ETSGLOBAL::main"
//! RUN_PGO_PAOC options: "--compiler-regex=ets_try_catch4.MultipleCatch::__noinline__multipleCatch"
//! METHOD       "ets_try_catch4.MultipleCatch::__noinline__multipleCatch"
//! PASS_AFTER   "Inline"
//! INST_COUNT   "Throw",2
//! PASS_AFTER   "TryCatchResolving"
//! INST_COUNT   "Throw",2

class Error0 extends Error {}
class Error1 extends Error {}

function throw_errors(i: int): void {
 switch (i) {
 case 0:
      throw new Error0();
 case 1:
     throw new Error1();
 default:
     break;
 }
}

class MultipleCatch {

  __noinline__multipleCatch() : int {
    let res : int = 0;
    for (let i : int = 0; i < 16; i++) {
      let k : int = i % 2;
      try {
        throw_errors(k);
      } catch (e) {
        if (e instanceof Error0) {
          res += 1;
        } else if (e instanceof Error1) {
          res += 2;
        } else {
          res += 100;
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
        res += mc.__noinline__multipleCatch();
    }
    return res - 240;
}
