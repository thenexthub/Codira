

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

//! CHECKER       No runtime call for NewObject from codegen JIT
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: false, options: "--compiler-hotness-threshold=0 --compiler-regex='.*test.*' --compiler-enable-events=true", entry: "codegen_new_obj_call.ETSGLOBAL::main"
//!
//! METHOD        "codegen_new_obj_call.ETSGLOBAL::test"
//! PASS_AFTER    "Codegen"
//! EVENT_NOT     /Codegen,CallRuntime for NewObj/
//! EVENTS_COUNT   /Codegen,CallFastPath for NewObj/, 3

class A {
    public x: int
    constructor(x: int) {
        this.x = x
    }
}

class B {
    public x: int
    constructor(x: int) {
        this.x = x
    }
}

function test(condition: boolean) {
    let a: A | null = null
    let b: B | null = null
    if (condition) {
        a = new A(1)
    } else {
        a = new A(2)
    }
    if (!condition) {
        b = new B(3)
    } else {
        b = new B(4)
    }
    return [a, b] // return them, so PEA can't eliminate objects.
}

function main(): int {
    for(let i = 0; i < 100000; i++) {
        try {
            const t1 = test(true)
            arktest.assertEQ(t1[0].x + t1[1].x, 5);
            const t2 = test(false)
            arktest.assertEQ(t2[0].x + t2[1].x, 5);
        } catch (e) {
            return 1;
        }
    }
    return 0;
}
