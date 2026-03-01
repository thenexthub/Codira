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

//! CHECKER      Memory Coalescing JIT
//! SKIP_IF      @architecture != "arm64"
//! RUN          force_jit: true, options: "--compiler-loop-unroll=false", entry: "memory_coalescing1.ETSGLOBAL::main"
//! EVENT        /Compilation,memory_coalescing1.ETSGLOBAL::main,.*,COMPILED/
//! EVENT        /Compilation,memory_coalescing1.Test::__noinline__fnSum,.*,COMPILED/
//! EVENT        /Compilation,memory_coalescing1.Test::__noinline__fnSumMem,.*,COMPILED/
//! EVENT        /Compilation,memory_coalescing1.Test::__noinline__fnSumGC,.*,COMPILED/
//! EVENT        /Compilation,memory_coalescing1.Test::__noinline__fnSumCls,.*,COMPILED/
//! METHOD        "memory_coalescing1.Test::__noinline__fnSum"
//! PASS_BEFORE  "MemoryCoalescing"
//! INST_NOT     "LoadArrayPair"
//! INST_NOT     "LoadPairPart"
//! INST_NOT     "StoreArrayPair"
//! PASS_AFTER   "MemoryCoalescing"
//! INST         "LoadArrayPair"
//! INST         "LoadPairPart"
//! INST         "StoreArrayPair"
//! INST_COUNT   /StoreObjectPair/, 2
//! INST_COUNT   /LoadObjectPair/, 1
//! METHOD       "memory_coalescing1.Test::__noinline__fnSumMem"
//! PASS_BEFORE  "MemoryCoalescing"
//! INST_NOT     "LoadArrayPair"
//! INST_NOT     "LoadPairPart"
//! INST_NOT     "StoreArrayPair"
//! PASS_AFTER   "MemoryCoalescing"
//! INST_NOT     "LoadArrayPair"
//! INST_NOT     "LoadPairPart"
//! INST_NOT     "StoreArrayPair"
//! INST_COUNT   /StoreObjectPair/, 1
//! INST_NOT     "LoadObjectPair"
//! METHOD       "memory_coalescing1.Test::__noinline__fnSumGC"
//! PASS_BEFORE  "MemoryCoalescing"
//! INST_NOT     "LoadArrayPair"
//! INST_NOT     "LoadPairPart"
//! INST_NOT     "StoreArrayPair"
//! PASS_AFTER   "MemoryCoalescing"
//! INST_NOT     "LoadArrayPair"
//! INST_NOT     "LoadPairPart"
//! INST_NOT     "StoreArrayPair"
//! INST_COUNT   /StoreObjectPair/, 1
//! INST_NOT     "LoadObjectPair"
//! METHOD       "memory_coalescing1.Test::__noinline__fnSumCls"
//! PASS_BEFORE  "MemoryCoalescing"
//! INST_NOT     "LoadArrayPair"
//! INST_NOT     "LoadPairPart"
//! INST_NOT     "StoreArrayPair"
//! PASS_AFTER   "MemoryCoalescing"
//! INST         "LoadArrayPair"
//! INST         "LoadPairPart"
//! INST_NOT     "StoreArrayPair"
//! INST_COUNT   /StoreObjectPair/, 1
//! INST_COUNT   /LoadObjectPair/, 1

class A {
    public a : int;
    public f : int;
    public g : int;
}

export class Test {

    private static __noinline__fnSum(u: FixedArray<double>, v: FixedArray<double>): void {
        let t: A = new A();
        t.f = 0;
        t.g = 0;
        for (let j: int = 0; j < u.length; j = j + 2) {
            t.f += u[j];
            t.g += u[j + 1];
        }
        for (let i: int = 0; i < v.length; i = i + 2) {
            v[i] = t.f;
            v[i + 1] = t.g + 1;
        }
    }

    private static __noinline__fnMem(u: FixedArray<double>, v: FixedArray<double>, i: int): void {
        u[i] = v[i];
    }

    // memory coalescing can't be performed over CallStatic that is barrier
    private static __noinline__fnSumMem(u: FixedArray<double>, v: FixedArray<double>): void {
        let t: A = new A();
        t.f = 0;
        t.g = 0;
        for (let j: int = 0; j < u.length; j = j + 2) {
            t.f += u[j];
            Test.__noinline__fnMem(u, v, j);
            t.g += u[j + 1];
        }
        for (let i: int = 0; i < v.length; i = i + 2) {
            v[i] = t.f;
            Test.__noinline__fnMem(u, v, i);
            v[i + 1] = t.g + 1;
        }
    }

    // memory coalescing can't be performed over GC call that is barrier
    private static __noinline__fnSumGC(u: FixedArray<double>, v: FixedArray<double>): void {
        try {
            let t: A = new A();
            t.f = 0;
            t.g = 0;
            for (let j: int = 0; j < u.length; j = j + 2) {
                t.f += u[j];
                GC.startGC(GC.Cause.FULL);
                t.g += u[j + 1];
            }
            for (let i: int = 0; i < v.length; i = i + 2) {
                v[i] = t.f;
                GC.startGC(GC.Cause.FULL);
                v[i + 1] = t.g + 1;
            }
        } catch (error) {
        }
    }

    // memory coalescing over NullCheck can be performed for Load and can't for Store
    private static __noinline__fnSumCls(u: FixedArray<double>, v: FixedArray<double>, o: A, d: int): void {
        let t: A = new A();
        t.f = 0;
        t.g = 0;
        for (let j: int = 0; j < u.length; j = j + 2) {
            t.f += u[j];
            o.a += v.length/(j + d);
            t.g += u[j + 1];
        }
        for (let i: int = 0; i < v.length; i = i + 2) {
            v[i] = t.f;
            o.a += v.length/(i + d);
            v[i + 1] = t.g + 1;
        }
    }

    __noinline__test(n: int): void {
        let i: int;
        let u: FixedArray<double> = new FixedArray<double>(n)
        let v: FixedArray<double> = new FixedArray<double>(n)
        let vv1: double = 0;
        let vv2: double = 0;
        let vv3: double = 0;
        let vv4: double = 0;
        for (i = 0; i < n; i++) {
            u[i] = i; v[i] = 0;
        }
        Test.__noinline__fnSum(u, v);
        for (i = 0; i < n; i++) {
            vv1 += v[i];
        }
        for (i = 0; i < n; i++) {
            u[i] = i; v[i] = 0;
        }
        Test.__noinline__fnSumGC(u, v);
        for (i = 0; i < n; i++) {
            vv2 += v[i];
        }
        for (i = 0; i < n; i++) {
            u[i] = i; v[i] = 0;
        }
        Test.__noinline__fnSumMem(u, v);
        for (i = 0; i < n; i++) {
            vv3 += v[i];
        }
        let a: A = new A();
        a.a = 100;
        for (i = 0; i < n; i++) {
            u[i] = i; v[i] = 0;
        }
        Test.__noinline__fnSumCls(u, v, a, a.a);
        for (i = 0; i < n; i++) {
            vv4 += v[i];
        }
        arktest.assertEQ(vv1, vv2)
        arktest.assertEQ(vv2, vv3)
        arktest.assertEQ(vv3, vv4)
    }
}

let s0 = new Test();

function main(): void {
    const n: int = 20;
    s0.__noinline__test(n);
}
