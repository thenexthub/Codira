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

import { min as minChar } from "std/math"

function testNoArguments() {
    arktest.assertEQ(Math.min(), Infinity)
}

function testSingleArgument() {
    arktest.assertEQ(Math.min(5), 5)
    arktest.assertEQ(Math.min(-5), -5)
    arktest.assertEQ(Math.min(0), 0)
    arktest.assertEQ(Math.min(Infinity), Infinity)
    arktest.assertEQ(Math.min(-Infinity), -Infinity)
    arktest.assertEQ(Math.min(3.14), 3.14)
    arktest.assertEQ(Math.min(-0), -0)
}

function testTwoArguments() {
    arktest.assertEQ(Math.min(1, 2), 1)
    arktest.assertEQ(Math.min(2, 1), 1)
    arktest.assertEQ(Math.min(-1, -2), -2)
    arktest.assertEQ(Math.min(-2, -1), -2)
    arktest.assertEQ(Math.min(0, 1), 0)
    arktest.assertEQ(Math.min(1, 0), 0)
}

function testMultipleArguments() {
    arktest.assertEQ(Math.min(1, 2, 3, 4, 5), 1)
    arktest.assertEQ(Math.min(5, 4, 3, 2, 1), 1)
    arktest.assertEQ(Math.min(3, 1, 4, 1, 5), 1)
    arktest.assertEQ(Math.min(-1, -5, -3, -2, -4), -5)
}

function testWithNaN() {
    arktest.assertTrue(Number.isNaN(Math.min(1, NaN, 3)))
    arktest.assertTrue(Number.isNaN(Math.min(NaN, 2)))
    arktest.assertTrue(Number.isNaN(Math.min(NaN)))
    arktest.assertTrue(Number.isNaN(Math.min(1, 2, NaN, 4)))
}

function testWithInfinity() {
    arktest.assertEQ(Math.min(Infinity, 1), 1)
    arktest.assertEQ(Math.min(1, Infinity), 1)
    arktest.assertEQ(Math.min(Infinity, Infinity), Infinity)
    arktest.assertEQ(Math.min(-Infinity, 1), -Infinity)
    arktest.assertEQ(Math.min(1, -Infinity), -Infinity)
    arktest.assertEQ(Math.min(Infinity, -Infinity), -Infinity)
}

function testWithZero() {
    arktest.assertEQ(Math.min(0, 0), 0)
    arktest.assertEQ(Math.min(0, -0), -0)
    arktest.assertEQ(Math.min(-0, 0), -0)
    arktest.assertEQ(Math.min(-0, -0), -0)
}

function testWithFloats() {
    arktest.assertEQ(Math.min(1.5, 2.5), 1.5)
    arktest.assertEQ(Math.min(2.5, 1.5), 1.5)
    arktest.assertEQ(Math.min(1.1, 1.2, 1.3), 1.1)
    arktest.assertEQ(Math.min(-1.5, -2.5), -2.5)
    arktest.assertEQ(Math.min(0.1, 0.2, 0.3), 0.1)
}

function testNegativeAndPositive() {
    arktest.assertEQ(Math.min(-5, 5), -5)
    arktest.assertEQ(Math.min(5, -5), -5)
    arktest.assertEQ(Math.min(-10, 0, 10), -10)
    arktest.assertEQ(Math.min(10, 0, -10), -10)
}

function testLargeNumbers() {
    arktest.assertEQ(Math.min(1e100, 1e101), 1e100)
    arktest.assertEQ(Math.min(Number.MAX_VALUE, Number.MAX_VALUE - 1), Number.MAX_VALUE - 1)
    arktest.assertEQ(Math.min(-Number.MAX_VALUE, -Number.MAX_VALUE + 1), -Number.MAX_VALUE)
}

function testSmallNumbers() {
    arktest.assertEQ(Math.min(1e-100, 1e-101), 1e-101)
    arktest.assertEQ(Math.min(Number.MIN_VALUE, Number.MIN_VALUE * 2), Number.MIN_VALUE)
}

function testMixedValues() {
    arktest.assertEQ(Math.min(100, -50, 0, 25, -75), -75)
    arktest.assertEQ(Math.min(1.5, -2.7, 0.5, -0.5), -2.7)
    arktest.assertEQ(Math.min(1000, 1, 100, 10), 1)
}

function testCharOverload() {
    let a: char = c'a'
    let b: char = c'z'
    let c: char = c'A'
    let d: char = c'Z'

    arktest.assertEQ(minChar(a, b), a)
    arktest.assertEQ(minChar(b, a), a)
    arktest.assertEQ(minChar(c, d), c)
    arktest.assertEQ(minChar(a, c), c)  // 'A' < 'a' in ASCII
    arktest.assertEQ(minChar(c'0', c'9'), c'0')
}

function testIntOverload() {
    let a: int = 10
    let b: int = 20
    let c: int = -5
    let d: int = 0

    arktest.assertEQ(Math.min(a, b), a)
    arktest.assertEQ(Math.min(b, a), a)
    arktest.assertEQ(Math.min(c, d), c)
    arktest.assertEQ(Math.min(a, c), c)
    arktest.assertEQ(Math.min(Int.MAX_VALUE, Int.MIN_VALUE), Int.MIN_VALUE)
}

function testLongOverload() {
    let a: long = 100000000000
    let b: long = 200000000000
    let c: long = -50000000000
    let d: long = 0

    arktest.assertEQ(Math.min(a, b), a)
    arktest.assertEQ(Math.min(b, a), a)
    arktest.assertEQ(Math.min(c, d), c)
    arktest.assertEQ(Math.min(a, c), c)
    arktest.assertEQ(Math.min(Long.MAX_VALUE, Long.MIN_VALUE), Long.MIN_VALUE)
}

function testFloatOverload() {
    let a: float = 1.5f
    let b: float = 2.5f
    let c: float = -1.5f
    let d: float = 0.0f

    arktest.assertEQ(Math.min(a, b), a)
    arktest.assertEQ(Math.min(b, a), a)
    arktest.assertEQ(Math.min(c, d), c)
    arktest.assertEQ(Math.min(a, c), c)
}

function testDoubleOverload() {
    let a: double = 1.5
    let b: double = 2.5
    let c: double = -1.5
    let d: double = 0.0

    arktest.assertEQ(Math.min(a, b), a)
    arktest.assertEQ(Math.min(b, a), a)
    arktest.assertEQ(Math.min(c, d), c)
    arktest.assertEQ(Math.min(a, c), c)
}

function main() {
    const suite = new arktest.ArkTestsuite("Math.min tests")
    suite.addTest("Test No Arguments", testNoArguments)
    suite.addTest("Test Single Argument", testSingleArgument)
    suite.addTest("Test Two Arguments", testTwoArguments)
    suite.addTest("Test Multiple Arguments", testMultipleArguments)
    suite.addTest("Test With NaN", testWithNaN)
    suite.addTest("Test With Infinity", testWithInfinity)
    suite.addTest("Test With Zero", testWithZero)
    suite.addTest("Test With Floats", testWithFloats)
    suite.addTest("Test Negative And Positive", testNegativeAndPositive)
    suite.addTest("Test Large Numbers", testLargeNumbers)
    suite.addTest("Test Small Numbers", testSmallNumbers)
    suite.addTest("Test Mixed Values", testMixedValues)
    suite.addTest("Test Char Overload", testCharOverload)
    suite.addTest("Test Int Overload", testIntOverload)
    suite.addTest("Test Long Overload", testLongOverload)
    suite.addTest("Test Float Overload", testFloatOverload)
    suite.addTest("Test Double Overload", testDoubleOverload)
    return suite.run()
}
