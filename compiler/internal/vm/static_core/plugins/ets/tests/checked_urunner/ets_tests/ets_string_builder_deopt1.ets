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

//! CHECKER      JIT optimize string builder instance, don't insert deoptimization
//! RUN          options: "--compiler-regex='.*ETSGLOBAL::(__noinline__caller|throws)' --no-async-jit=true --compiler-hotness-threshold=50 --compiler-profiling-threshold=0", entry: "ets_string_builder_deopt1.ETSGLOBAL::main"
//! EVENT        /Inline,ets_string_builder_deopt1.ETSGLOBAL::__noinline__caller,ets_string_builder_deopt1.ETSGLOBAL::throws,.*,STATIC,SUCCESS/
//! METHOD       "throws"
//! PASS_BEFORE  "BranchElimination"
//! INST_COUNT   "NewObject", 3
//! PASS_AFTER   "SimplifyStringBuilder"
//! INST_COUNT   "NewObject", 1
//! METHOD       "__noinline__caller"
//! PASS_AFTER   "TryCatchResolving"
//! INST_NOT     "DeoptimizeIf"

//! CHECKER      AOT optimize string builder instance, don't insert deoptimization
//! SKIP_IF      @architecture == "arm32"
//! RUN_PAOC     options: ""
//! EVENT        /Inline,ets_string_builder_deopt1.ETSGLOBAL::__noinline__caller,ets_string_builder_deopt1.ETSGLOBAL::throws,.*,STATIC,SUCCESS/
//! METHOD       "throws"
//! PASS_BEFORE  "BranchElimination"
//! INST_COUNT   "NewObject", 3
//! PASS_AFTER   "SimplifyStringBuilder"
//! INST_COUNT   "NewObject", 1
//! METHOD       "__noinline__caller"
//! PASS_AFTER   "TryCatchResolving"
//! INST_NOT     "DeoptimizeIf"
//! RUN          entry: "ets_string_builder_deopt1.ETSGLOBAL::main"

//! CHECKER      AOT PGO optimize string builder instance, can insert deoptimization
//! SKIP_IF      @architecture == "arm32"
//! RUN_PGO_PROF entry: "ets_string_builder_deopt1.ETSGLOBAL::main -- x"
//! RUN_PGO_PAOC options: ""
//! EVENT        /Inline,ets_string_builder_deopt1.ETSGLOBAL::__noinline__caller,ets_string_builder_deopt1.ETSGLOBAL::throws,.*,STATIC,SUCCESS/
//! METHOD       "throws"
//! PASS_BEFORE  "BranchElimination"
//! INST_COUNT   "NewObject", 3
//! PASS_AFTER   "SimplifyStringBuilder"
//! INST_COUNT   "NewObject", 1
//! METHOD       "__noinline__caller"
//! PASS_AFTER   "TryCatchResolving"
//! # Here, used SaveState of `LoadAndInitClass 'std.core.StringBuilder'` to deoptimize, which doesn't need the instance
//! INST         "DeoptimizeIf"
//! RUN          entry: "ets_string_builder_deopt1.ETSGLOBAL::main"

function throws(str: string, arg: int) {
  if (arg > 0) {
    // if:
    //  - StringBuilder instance if optimized out;
    //  - and deoptimization on throw is inserted;
    //  - and it occurs
    // there would be NullPointerError for StringBuilder instance.
    // assert that it doesn't happen
    throw new Error(str + 'ok');
  } else {
    return str + 'ok';
  }
}

function __noinline__caller(str: string, arg: int) {
  try {
    return throws(str, arg);
  } catch (e) {
    // if NullPointerError occurs, returns empty string
    return e.message;
  }
}

function main(args: FixedArray<string>): void {
  // check that concatenation succeeded and NPE didn't occur
  if (args.length > 0 && args[0] == 'x') {
    // AOT profile collection
    for (let i = 0; i < 200; i++) {
      arktest.assertEQ(__noinline__caller('maybe', 0), 'maybeok');
    }
  } else {
    // otherwise
    for (let i = 0; i < 200; i++) {
      arktest.assertEQ(__noinline__caller('maybe', i < 100 ? 0 : 1), 'maybeok');
    }
  }
}
