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

//! CHECKER      JIT leave string builder instance for proper inlining deoptimization
//! RUN          options: "--compiler-regex='.*ETSGLOBAL::__noinline__caller' --no-async-jit=true --compiler-hotness-threshold=50 --compiler-profiling-threshold=0", entry: "ets_string_builder_deopt3.ETSGLOBAL::main"
//! METHOD       "__noinline__caller"
//! PASS_BEFORE  "BranchElimination"
//! INST_COUNT   "NewObject", 1
//! PASS_AFTER   "SimplifyStringBuilder"
//! INST_COUNT   "NewObject", 0
//! INST_COUNT   "Intrinsic.StdCoreStringConcat2", 1
//! EVENT        /Inline,ets_string_builder_deopt3.ETSGLOBAL::__noinline__caller,ets_string_builder_deopt3.B::f,.*,SUCCESS/
//! PASS_AFTER   "Inline"
//! INST         "DeoptimizeIf"

//! CHECKER      AOT leave string builder instance for proper inlining deoptimization, but don't inline
//! SKIP_IF      @architecture == "arm32"
//! RUN_PAOC     options: ""
//! METHOD       "__noinline__caller"
//! PASS_BEFORE  "BranchElimination"
//! INST_COUNT   "NewObject", 1
//! PASS_AFTER   "SimplifyStringBuilder"
//! INST_COUNT   "NewObject", 0
//! INST_COUNT   "Intrinsic.StdCoreStringConcat2", 1
//! EVENT_NOT    /Inline,ets_string_builder_deopt3.ETSGLOBAL::__noinline__caller,ets_string_builder_deopt3.B::f,.*,SUCCESS/
//! PASS_AFTER   "Inline"
//! INST_NOT     "DeoptimizeIf"
//! RUN          entry: "ets_string_builder_deopt3.ETSGLOBAL::main"

//! CHECKER      AOT PGO leave string builder instance for proper inlining deoptimization
//! SKIP_IF      @architecture == "arm32"
//! RUN_PGO_PROF entry: "ets_string_builder_deopt3.ETSGLOBAL::main"
//! RUN_PGO_PAOC options: ""
//! METHOD       "__noinline__caller"
//! PASS_BEFORE  "BranchElimination"
//! INST_COUNT   "NewObject", 1
//! PASS_AFTER   "SimplifyStringBuilder"
//! INST_COUNT   "NewObject", 0
//! INST_COUNT   "Intrinsic.StdCoreStringConcat2", 1
//! EVENT        /Inline,ets_string_builder_deopt3.ETSGLOBAL::__noinline__caller,ets_string_builder_deopt3.B::f,.*,SUCCESS/
//! PASS_AFTER   "Inline"
//! INST         "DeoptimizeIf"
//! RUN          entry: "ets_string_builder_deopt3.ETSGLOBAL::main"

class A {
  f(): string {
    return 'A'
  }
}

class B extends A {
  f(): string {
    return 'B'
  }
}

function __noinline__caller(obj: A, str: string) {
  // if:
  //  - StringBuilder instance if optimized out;
  //  - and deoptimization on non-matching object instance for inlining is inserted;
  //  - and it occurs
  // there would be NullPointerError for StringBuilder instance.
  // assert that it doesn't happen
  return obj.f() + str;
}

function main(args: FixedArray<string>): void {
  // check that concatenation succeeded and NPE didn't occur
  if (args.length > 0 && args[0] == 'x') {
    // AOT profile collection
    for (let i = 0; i < 200; i++) {
      arktest.assertEQ(__noinline__caller(new B(), `${i}`), `B${i}`);
    }
  } else {
    // otherwise
    for (let i = 0; i < 200; i++) {
      arktest.assertEQ(__noinline__caller(i < 100 ? new B() : new A(), `${i}`), i < 100 ? `B${i}` : `A${i}`);
    }
  }
}
