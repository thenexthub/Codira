/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

//! CHECKER       AOT PGO: no INLINE_IC deoptimization
//! SKIP_IF       @architecture == "arm32"
//! RUN_PGO_PROF  entry: "ext_inlining_inline_ic_no_deopt.ETSGLOBAL::main"
//! RUN_PGO_PAOC  options: "--compiler-inline-external-methods-aot=true --compiler-regex='.*__noinline__bar.*'"
//! METHOD        "ext_inlining_inline_ic_no_deopt.ETSGLOBAL::__noinline__bar"
//! PASS_AFTER    "Inline"
//! INST          /GetInstanceClass/
//! PASS_AFTER    "Codegen"
//! INST          /GetInstanceClass/
//! INST_NOT      /LoadImmediate\(class:/
//! RUN           entry: "ext_inlining_inline_ic_no_deopt.ETSGLOBAL::main", options: "--compiler-regex='.*main.*'"
//! EVENT_NOT     /DeoptimizationReason,.*,INLINE_IC/

let counter: int = 0;

class A {
    foo() { counter += 1; }
}
class B extends A {
    foo() { counter += 2; }
}

function __noinline__foo(): A {
    return new B();
}

function __noinline__bar() {
    const a = __noinline__foo();
    for (let i = 0; i < 10; i++) {
        a.foo();
    }
}

function main() {
    __noinline__bar();
    arktest.assertEQ(counter, 20);
}
