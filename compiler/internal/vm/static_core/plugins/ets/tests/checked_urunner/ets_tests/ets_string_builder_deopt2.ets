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

//! CHECKER      JIT leave string builder instance for safe inlining
//! RUN          options: "--compiler-regex='.*ETSGLOBAL::__noinline__caller' --no-async-jit=true --compiler-hotness-threshold=50 --compiler-profiling-threshold=0", entry: "ets_string_builder_deopt2.ETSGLOBAL::main"
//! METHOD       "__noinline__caller"
//! PASS_BEFORE  "BranchElimination"
//! INST_COUNT   "NewObject", 3
//! PASS_AFTER   "SimplifyStringBuilder"
//! INST_COUNT   "NewObject", 1
//! INST_COUNT   "Intrinsic.StdCoreStringConcat2", 2
//! PASS_AFTER   "TryCatchResolving"
//! INST         "DeoptimizeIf"

//! CHECKER      AOT leave string builder instance for safe inlining, no deoptimize without PGO
//! SKIP_IF      @architecture == "arm32"
//! RUN_PAOC     options: ""
//! METHOD       "__noinline__caller"
//! PASS_BEFORE  "BranchElimination"
//! INST_COUNT   "NewObject", 3
//! PASS_AFTER   "SimplifyStringBuilder"
//! INST_COUNT   "NewObject", 1
//! INST_COUNT   "Intrinsic.StdCoreStringConcat2", 2
//! PASS_AFTER   "TryCatchResolving"
//! INST_NOT     "DeoptimizeIf"
//! RUN          entry: "ets_string_builder_deopt2.ETSGLOBAL::main"

//! CHECKER      AOT PGO leave string builder instance for safe inlining
//! SKIP_IF      @architecture == "arm32"
//! RUN_PGO_PROF entry: "ets_string_builder_deopt2.ETSGLOBAL::main -- x"
//! RUN_PGO_PAOC options: ""
//! METHOD       "__noinline__caller"
//! PASS_BEFORE  "BranchElimination"
//! INST_COUNT   "NewObject", 3
//! PASS_AFTER   "SimplifyStringBuilder"
//! INST_COUNT   "NewObject", 1
//! INST_COUNT   "Intrinsic.StdCoreStringConcat2", 2
//! PASS_AFTER   "TryCatchResolving"
//! INST         "DeoptimizeIf"
//! RUN          entry: "ets_string_builder_deopt2.ETSGLOBAL::main"

function __noinline__callee(str: string): string {
  return str;
}

function __noinline__caller(str: string, arg: int) {
  try {
    if (arg > 0) {
      // if:
      //  - StringBuilder instance if optimized out;
      //  - and deoptimization on throw is inserted;
      //  - and it occurs
      // there would be NullPointerError for StringBuilder instance.
      // assert that it doesn't happen
      throw new Error(__noinline__callee(str) + 'ok');
    } else {
      return __noinline__callee(str) + 'ok';
    }
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
