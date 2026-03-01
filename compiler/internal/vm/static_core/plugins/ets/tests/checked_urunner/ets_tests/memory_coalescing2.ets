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

//! CHECKER      Memory Coalescing object fields of various types and sizes
//! SKIP_IF      @architecture != "arm64"
//! RUN          force_jit: true, options: "--compiler-loop-unroll=false", entry: "memory_coalescing2.ETSGLOBAL::main"
//! EVENT        /Compilation,memory_coalescing2.ETSGLOBAL::main,.*,COMPILED/
//! EVENT        /Compilation,memory_coalescing2.Test::__noinline__fnSumII,.*,COMPILED/
//! METHOD        "memory_coalescing2.Test::__noinline__fnSumII"
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
//! METHOD        "memory_coalescing2.Test::__noinline__fnSumIF"
//! PASS_BEFORE  "MemoryCoalescing"
//! INST_NOT     "LoadArrayPair"
//! INST_NOT     "LoadPairPart"
//! INST_NOT     "StoreArrayPair"
//! PASS_AFTER   "MemoryCoalescing"
//! INST         "LoadArrayPair"
//! INST         "LoadPairPart"
//! INST         "StoreArrayPair"
//! INST_NOT     "StoreObjectPair"
//! INST_NOT     "LoadObjectPair"
//! METHOD        "memory_coalescing2.Test::__noinline__fnSumFF"
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
//! METHOD        "memory_coalescing2.Test::__noinline__fnSumLL"
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
//! METHOD        "memory_coalescing2.Test::__noinline__fnSumLD"
//! PASS_BEFORE  "MemoryCoalescing"
//! INST_NOT     "LoadArrayPair"
//! INST_NOT     "LoadPairPart"
//! INST_NOT     "StoreArrayPair"
//! PASS_AFTER   "MemoryCoalescing"
//! INST         "LoadArrayPair"
//! INST         "LoadPairPart"
//! INST         "StoreArrayPair"
//! INST_NOT     "StoreObjectPair"
//! INST_NOT     "LoadObjectPair"
//! METHOD        "memory_coalescing2.Test::__noinline__fnSumDD"
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

class A {
    public i1 : int;
    public i2 : int;
    public f1 : float;
    public f2 : float;
    public l1 : long;
    public l2 : long
    public d1 : double;
    public d2 : double;
}

export class Test {

    private static __noinline__fnSumII(u: FixedArray<double>, v: FixedArray<double>): void {
        let t: A = new A();
        t.i1 = 0;
        t.i2 = 0;
        for (let j: int = 0; j < u.length; j = j + 2) {
            t.i1 += u[j];
            t.i2 += u[j + 1];
        }
        for (let i: int = 0; i < v.length; i = i + 2) {
            v[i] = t.i1;
            v[i + 1] = t.i2 + 1;
        }
    }

    private static __noinline__fnSumIF(u: FixedArray<double>, v: FixedArray<double>): void {
        let t: A = new A();
        t.i2 = 0;
        t.f1 = 0;
        for (let j: int = 0; j < u.length; j = j + 2) {
            t.i2 += u[j];
            t.f1 += u[j + 1];
        }
        for (let i: int = 0; i < v.length; i = i + 2) {
            v[i] = t.i2;
            v[i + 1] = t.f1 + 1;
        }
    }

    private static __noinline__fnSumFF(u: FixedArray<double>, v: FixedArray<double>): void {
        let t: A = new A();
        t.f1 = 0;
        t.f2 = 0;
        for (let j: int = 0; j < u.length; j = j + 2) {
            t.f1 += u[j];
            t.f2 += u[j + 1];
        }
        for (let i: int = 0; i < v.length; i = i + 2) {
            v[i] = t.f1;
            v[i + 1] = t.f2 + 1;
        }
    }

    private static __noinline__fnSumLL(u: FixedArray<double>, v: FixedArray<double>): void {
        let t: A = new A();
        t.l1 = 0;
        t.l2 = 0;
        for (let j: int = 0; j < u.length; j = j + 2) {
            t.l1 += u[j];
            t.l2 += u[j + 1];
        }
        for (let i: int = 0; i < v.length; i = i + 2) {
            v[i] = t.l1;
            v[i + 1] = t.l2 + 1;
        }
    }

    private static __noinline__fnSumLD(u: FixedArray<double>, v: FixedArray<double>): void {
        let t: A = new A();
        t.l2 = 0;
        t.d1 = 0;
        for (let j: int = 0; j < u.length; j = j + 2) {
            t.l2 += u[j];
            t.d1 += u[j + 1];
        }
        for (let i: int = 0; i < v.length; i = i + 2) {
            v[i] = t.l2;
            v[i + 1] = t.d1 + 1;
        }
    }

    private static __noinline__fnSumDD(u: FixedArray<double>, v: FixedArray<double>): void {
        let t: A = new A();
        t.d1 = 0;
        t.d2 = 0;
        for (let j: int = 0; j < u.length; j = j + 2) {
            t.d1 += u[j];
            t.d2 += u[j + 1];
        }
        for (let i: int = 0; i < v.length; i = i + 2) {
            v[i] = t.d1;
            v[i + 1] = t.d2 + 1;
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
        let vv5: double = 0;
        let vv6: double = 0;
        for (i = 0; i < n; i++) {
            u[i] = i; v[i] = 0;
        }
        Test.__noinline__fnSumII(u, v);
        for (i = 0; i < n; i++) {
            vv1 += v[i];
        }
        for (i = 0; i < n; i++) {
            u[i] = i; v[i] = 0;
        }
        Test.__noinline__fnSumIF(u, v);
        for (i = 0; i < n; i++) {
            vv2 += v[i];
        }
        for (i = 0; i < n; i++) {
            u[i] = i; v[i] = 0;
        }
        Test.__noinline__fnSumFF(u, v);
        for (i = 0; i < n; i++) {
            vv3 += v[i];
        }
        for (i = 0; i < n; i++) {
            u[i] = i; v[i] = 0;
        }
        Test.__noinline__fnSumLL(u, v);
        for (i = 0; i < n; i++) {
            vv4 += v[i];
        }
        for (i = 0; i < n; i++) {
            u[i] = i; v[i] = 0;
        }
        Test.__noinline__fnSumLD(u, v);
        for (i = 0; i < n; i++) {
            vv5 += v[i];
        }
        for (i = 0; i < n; i++) {
            u[i] = i; v[i] = 0;
        }
        Test.__noinline__fnSumDD(u, v);
        for (i = 0; i < n; i++) {
            vv6 += v[i];
        }
        arktest.assertEQ(vv1, vv2)
        arktest.assertEQ(vv2, vv3)
        arktest.assertEQ(vv3, vv4)
        arktest.assertEQ(vv4, vv5)
        arktest.assertEQ(vv5, vv6)
    }
}

let s0 = new Test();

function main(): void {
    const n: int = 20;
    s0.__noinline__test(n);
}
