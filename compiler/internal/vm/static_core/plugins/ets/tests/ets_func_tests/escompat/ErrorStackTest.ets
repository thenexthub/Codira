/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

function test(result: boolean, name: String): int {
    if (result) {
        console.log("PASSED: " + name)
        return 0
    }
    console.log("FAILED: " + name)
    return 1
}

function testTrace(name: String, fun: () => void, expected: string[]): int {
    let threw = false
    let failures = 0
    try {
        fun()
    } catch (e) {
        for (let i = 0; i < expected.length; ++i) {
            failures += test((e as Error).stack!.indexOf(expected[i]) != -1,
                        name + " has to contain expected[" + i + "] stack = " + (e as Error).stack)
        }
    }
    threw = true;
    failures += test(threw, name + " threw")
    return failures
}

function testArrayNative() {
    let array = Array.of<number>(1, 2, 3)
    array.forEach((v: number): void => { throw new Error("FAIL") })
}

class Nirk {
    override toString(): String {
        throw new Error("FAIL")
    }
}

class Plonk {
    constructor() {
        throw new Error("FAIL")
    }
}

function outerFoo() {
    foo()
}

function foo() {
    throw new Error("FAIL")
}

function rec(n : int) {
    if (n == 0) {
        throw new Error("FAIL")
    } else {
        rec(n - 1)
    }
}

function testImplicitConversion() {
    let cl = new Nirk();
    let str : String = "a" + cl;
    arktest.assertTrue((!!str))
}

function testConstructor() {
    let cl = new Plonk()
    arktest.assertTrue((!!cl))
}

function testNestedFunc() {
    outerFoo()
}

function testRecursion() {
    rec(2)
}

function main(): int {
    let failures = 0;
    failures += testTrace("testArrayNative", testArrayNative, ["Array.forEach"])
    failures += testTrace("testImplicitConversion", testImplicitConversion, ["Nirk.toString"])
    failures += testTrace("testConstructor", testConstructor, ["Plonk.<ctor>"])
    failures += testTrace("testNestedFunc", testNestedFunc, ["foo", "outerFoo"])
    failures += testTrace("testRecursion", testRecursion, ["rec"])

    return failures
}
