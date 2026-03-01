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

// These tests were disabled because of catch block no longer accepting specific error type
//! DISABLED_CHECKER      Try Catch Resolving JIT Optimize with partial and default catches
//! RUN          force_jit: true, options: "--compiler-hotness-threshold=20 --compiler-profiling-threshold=0 --compiler-enable-osr=false --compiler-regex=ets_try_catch2.MultipleCatch::__noinline__multipleCatch", entry: "ets_try_catch2.ETSGLOBAL::main"
//! EVENT        /Compilation,ets_try_catch2.MultipleCatch::__noinline__multipleCatch,.*,COMPILED/
//! METHOD       "ets_try_catch2.MultipleCatch::__noinline__multipleCatch"
//! PASS_AFTER   "Inline"
//! INST_COUNT   "Throw",2
//! PASS_AFTER   "TryCatchResolving"
//! INST_NOT     "Throw"

//! DISABLED_CHECKER      Try Catch Resolving AOT Optimize with partial and default catches
//! SKIP_IF      @architecture == "arm32"
//! RUN_PAOC     options: "--compiler-regex=ets_try_catch2.MultipleCatch::__noinline__multipleCatch"
//! METHOD       "ets_try_catch2.MultipleCatch::__noinline__multipleCatch"
//! PASS_AFTER   "Inline"
//! INST_COUNT   "Throw",2
//! PASS_AFTER   "TryCatchResolving"
//! INST_COUNT   "Throw",2

//! DISABLED_CHECKER      Try Catch Resolving AOT+PGO Optimize with partial and default catches
//! SKIP_IF      @architecture == "arm32"
//! RUN_PGO_PROF entry: "ets_try_catch2.ETSGLOBAL::main"
//! RUN_PGO_PAOC options: "--compiler-regex=ets_try_catch2.MultipleCatch::__noinline__multipleCatch"
//! METHOD       "ets_try_catch2.MultipleCatch::__noinline__multipleCatch"
//! PASS_AFTER   "Inline"
//! INST_COUNT   "Throw",2
//! PASS_AFTER   "TryCatchResolving"
//! INST_NOT     "Throw"

class Error0 extends Error {}
class Error1 extends Error {}

class MultipleCatch {
  __noinline__multipleCatch() : int {
    let res : int = 0;
    try {
        for (let i : int = 0; i < 16; i++) {
            try {
                switch (i) {
                case 0: case 1: case 2: case 3: case 4:
                case 5: case 6: case 7: case 8: case 9:
                    throw new Error0();
                case 10:
                    throw new Error1();
                }
            } catch (e) {
                if (e instanceof Error0) {
                    res += 1;
                } else {
                    throw e;
                }
            }
        }
    } catch (e) {
        res += 100;
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
    return res - 1100;
}
