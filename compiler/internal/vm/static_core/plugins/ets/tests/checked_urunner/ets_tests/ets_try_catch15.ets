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

//! CHECKER      Try Catch Resolving JIT IfImm is replaced by DeoptimizeIf 
//! RUN          force_jit: true, options: "--compiler-hotness-threshold=5 --compiler-profiling-threshold=0 --compiler-enable-osr=false --compiler-regex=ets_try_catch15.ETSGLOBAL::__noinline__test", entry: "ets_try_catch15.ETSGLOBAL::main"
//! EVENT        /Compilation,ets_try_catch15.ETSGLOBAL::__noinline__test,.*,COMPILED/
//! METHOD       "ets_try_catch15.ETSGLOBAL::__noinline__test"
//! PASS_AFTER   "Inline"
//! INST_COUNT   "Throw", 2
//! PASS_AFTER   "TryCatchResolving"
//! INST_COUNT   "Throw", 1
//! INST         "DeoptimizeIf"

//! CHECKER      Try Catch Resolving AOT IfImm is not replaced by DeoptimizeIf
//! SKIP_IF      @architecture == "arm32"
//! RUN_PAOC     options: "--compiler-regex=ets_try_catch15.ETSGLOBAL::__noinline__test"
//! METHOD       "ets_try_catch15.ETSGLOBAL::__noinline__test"
//! PASS_AFTER   "Inline"
//! INST         "Throw"
//! PASS_AFTER   "TryCatchResolving"
//! INST         "Throw"
//! INST_NOT     "DeoptimizeIf"

//! CHECKER      Try Catch Resolving AOT+PGO both IfImms are replaced by DeoptimizeIf because first stmt in try-block is SaveState (SaveState+LoadAndInitClass is not optimized into LoadImmediate)
//! SKIP_IF      @architecture == "arm32"
//! RUN_PGO_PROF entry: "ets_try_catch15.ETSGLOBAL::main"
//! RUN_PGO_PAOC options: "--compiler-regex=ets_try_catch15.ETSGLOBAL::__noinline__test"
//! METHOD       "ets_try_catch15.ETSGLOBAL::__noinline__test"
//! PASS_AFTER   "Inline"
//! INST_COUNT   "Throw", 2
//! PASS_AFTER   "TryCatchResolving"
//! INST_NOT     "Throw"
//! INST         "DeoptimizeIf"

class Ex extends Error {}
let ex: Ex = new Ex()

class A {
    t: string = ""
    getType() : string {
        return this.t
    }
    constructor(p: int) { if (p == 0) throw ex; else this.t = "int" }
}

function __noinline__test() {
    try {
        if (new A(-1).getType() != "int") {
            throw Error()
        }
    } catch (e) {
        arktest.assertTrue(false);
    }
}

function main() {
    for (let i:int = 1; i < 20; ++i) {
       __noinline__test();
    }
}