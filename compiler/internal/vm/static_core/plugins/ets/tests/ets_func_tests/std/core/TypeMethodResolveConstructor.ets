/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

let called = -1

interface A {}
class B implements A {}
class D implements A {}

const AType = Type.resolve("TypeMethodResolveConstructor.A") as InterfaceType
const BType = Type.of(new B()) as ClassType
const CType = Type.of(new C()) as ClassType

class C {
    constructor() {
        called = 0
    }

    constructor(a: int) {
        called = 1
    }

    constructor(a: A) {
        called = 11
    }
    constructor(a: B) {
        called = 12
    }

    constructor(a: int, b: int) {
        called = 2
    }
}

function test(args: FixedArray<Any>, exp: int): int {
    called = -1
    CType.make(args)
    if (called != exp) {
        console.println("FAILED: " + exp + " got " + called)
        console.println(args)
        return 1
    } else {
        console.log("ok " + exp)
    }
    return 0
}

function mk(num: int, fn: (idx: int) => Any): FixedArray<Any> {
    const arr : FixedArray<Any> = new FixedArray<Any>(num)
    for (let i = 0; i < num; i++) {
        arr[i] = fn(i)
    }
    return arr;
}

function from(x: Any): FixedArray<Any> {
    const ret :FixedArray<Any> = new FixedArray<Any>(1)
    ret[0] = x
    return ret
}

function main(): int {
    let f = 0
    f += test(new FixedArray<Any>(0), 0)
    f += test(mk(2, (idx: int): Any => { return idx as Int }), 2)
    f += test(from(new B()), 12)
    f += test(from(new D()), 11)

    let threw = false
    try {
        CType.make(from(new C()))
    } catch (e) {
        threw = true
    }
    if (!threw) {
        f += 1
        console.println("error not thrown")
    }
    return f
}
