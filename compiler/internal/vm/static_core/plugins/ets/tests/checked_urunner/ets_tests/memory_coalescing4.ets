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

//! CHECKER      Memory Coalescing checking may alias objects
//! SKIP_IF      @architecture != "arm64"
//! RUN          force_jit: true, options: "--compiler-loop-unroll=false", entry: "memory_coalescing4.ETSGLOBAL::main"
//! EVENT        /Compilation,memory_coalescing4.ETSGLOBAL::main,.*,COMPILED/
//! EVENT        /Compilation,memory_coalescing4.Test::__noinline__fnSum,.*,COMPILED/
//! METHOD        "memory_coalescing4.Test::__noinline__fnSum"
//! PASS_BEFORE  "MemoryCoalescing"
//! INST_NOT     "StoreArrayPair"
//! INST_NOT     "LoadObjectPair"
//! PASS_AFTER   "MemoryCoalescing"
//! INST         "StoreArrayPair"
//! INST_NOT     "LoadObjectPair"

class A {
    public i1 : int;
    public i2 : int;
}

export class Test {

    private static __noinline__fnSum(u: FixedArray<int>,
                                     a1: A,
                                     a2: A): void {
        a1.i1 = 1;
        a2.i2 = 2;
        a1.i2 = 3;
        a2.i2 = 4;
        for (let i: int = 0; i < u.length; i = i + 2) {
            u[i] = a1.i1;
            a2.i1 = u[i];
            u[i + 1] = a1.i2;
        }
    }

    __noinline__test(n: int): void {
        let i: int;
        let u: FixedArray<int> = new FixedArray<int>(n)
        let a: A = new A();
        for (i = 0; i < n; i++) {
            u[i] = i;
        }
        Test.__noinline__fnSum(u, a, a);
    }
}

let s0 = new Test();

function main(): void {
    const n: int = 20;
    s0.__noinline__test(n);
}
