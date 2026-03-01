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

interface I {}
interface J {}
class A implements I {}
class B implements J {}
class C implements I, J {}

const TestType = Type.of(new Test()) as ClassType

class Test {
    constructor() {}

    constructor(x: I) {
        called = 1
    }

    constructor(x: J) {
        called = 2
    }
}

function test(args: FixedArray<Any>, exp: int): int {
    called = -1
    try {
        TestType.make(args)
    } catch (e) {
        console.println(e.toString())
        called = -1
    }
    if (called != exp) {
        console.println("FAILED: " + exp + " got " + called)
        console.println(args)
        return 1
    } else {
        console.log("ok " + exp)
    }
    return 0
}

function from(x: Any): FixedArray<Any> {
    const ret : FixedArray<Any> = new FixedArray<Any>(1)
    ret[0] = x
    return ret
}

function main(): int {
    let f = 0
    f += test(from(new A()), 1)
    f += test(from(new B()), 2)

    let threw = false
    try {
        TestType.make(from(new C()))
    } catch (e) {
        threw = true
    }
    if (!threw) {
        f += 1
        console.println("error not thrown")
    }
    return f
}
