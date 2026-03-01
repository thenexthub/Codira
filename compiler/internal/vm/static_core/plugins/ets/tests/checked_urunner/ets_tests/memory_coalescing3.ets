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
//! RUN          force_jit: true, options: "--compiler-loop-unroll=false", entry: "memory_coalescing3.ETSGLOBAL::main"
//! EVENT        /Compilation,memory_coalescing3.ETSGLOBAL::main,.*,COMPILED/
//! EVENT        /Compilation,memory_coalescing3.Test::__noinline__fnSum,.*,COMPILED/
//! METHOD        "memory_coalescing3.Test::__noinline__fnSum"
//! PASS_BEFORE  "MemoryCoalescing"
//! INST_NOT     "LoadArrayPair"
//! INST_NOT     "LoadPairPart"
//! INST_NOT     "StoreArrayPair"
//! PASS_AFTER   "MemoryCoalescing"
//! INST_NOT     "LoadArrayPair"
//! INST         "LoadPairPart"
//! INST_NOT     "StoreArrayPair"
//! INST_NOT     "StoreObjectPair"
//! INST         "LoadObjectPair"

class A {
    public i1 : int;
    public i2 : int;
}

export class Test {

    private static __noinline__fnSum(u: double[],
                                     v1: double[],
                                     v2: double[],
                                     a1: A,
                                     a2: A): void {
        a1.i1 = 1;
        a2.i2 = 2;
        a1.i2 = 3;
        a2.i2 = 4;
        for (let j: int = 0; j < u.length; j = j + 2) {
            a1.i1 += u[j];
            a2.i1 += u[j + 1];
            a1.i2 += u[j + 1];
            a2.i2 += u[j];
        }
        for (let i: int = 0; i < v1.length; i = i + 2) {
            v1[i] = a1.i1;
            v2[i] = a2.i1;
            v1[i + 1] = a1.i2 + 1;
            v2[i + 1] = a2.i2 + 1;
        }
    }

    __noinline__test(n: int): void {
        let i: int;
        let u: double[] = new double[n];
        let v1: double[] = new double[n];
        let v2: double[] = new double[n];
        let vv: double = 0;
        let a: A = new A();
        for (i = 0; i < n; i++) {
            u[i] = i; v1[i] = 0; v2[i] = 0;
        }
        Test.__noinline__fnSum(u, v1, v2, a, a);
        for (i = 0; i < n; i++) {
            vv += v1[i] + v2[i];
        }
        arktest.assertEQ(vv, 7720)
    }
}

let s0 = new Test();

function main(): void {
    const n: int = 20;
    s0.__noinline__test(n);
}
