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

import { max as maxChar } from "std/math"

function testNoArguments() {
    arktest.assertEQ(Math.max(), -Infinity)
}

function testSingleArgument() {
    arktest.assertEQ(Math.max(5), 5)
    arktest.assertEQ(Math.max(-5), -5)
    arktest.assertEQ(Math.max(0), 0)
    arktest.assertEQ(Math.max(Infinity), Infinity)
    arktest.assertEQ(Math.max(-Infinity), -Infinity)
    arktest.assertEQ(Math.max(3.14), 3.14)
    arktest.assertEQ(Math.max(-0), -0)
}

function testTwoArguments() {
    arktest.assertEQ(Math.max(1, 2), 2)
    arktest.assertEQ(Math.max(2, 1), 2)
    arktest.assertEQ(Math.max(-1, -2), -1)
    arktest.assertEQ(Math.max(-2, -1), -1)
    arktest.assertEQ(Math.max(0, 1), 1)
    arktest.assertEQ(Math.max(1, 0), 1)
}

function testMultipleArguments() {
    arktest.assertEQ(Math.max(1, 2, 3, 4, 5), 5)
    arktest.assertEQ(Math.max(5, 4, 3, 2, 1), 5)
    arktest.assertEQ(Math.max(3, 1, 4, 1, 5), 5)
    arktest.assertEQ(Math.max(-1, -5, -3, -2, -4), -1)
}

function testWithNaN() {
    arktest.assertTrue(Number.isNaN(Math.max(1, NaN, 3)))
    arktest.assertTrue(Number.isNaN(Math.max(NaN, 2)))
    arktest.assertTrue(Number.isNaN(Math.max(NaN)))
    arktest.assertTrue(Number.isNaN(Math.max(1, 2, NaN, 4)))
}

function testWithInfinity() {
    arktest.assertEQ(Math.max(Infinity, 1), Infinity)
    arktest.assertEQ(Math.max(1, Infinity), Infinity)
    arktest.assertEQ(Math.max(Infinity, Infinity), Infinity)
    arktest.assertEQ(Math.max(-Infinity, 1), 1)
    arktest.assertEQ(Math.max(1, -Infinity), 1)
    arktest.assertEQ(Math.max(Infinity, -Infinity), Infinity)
}

function testWithZero() {
    arktest.assertEQ(Math.max(0, 0), 0)
    arktest.assertEQ(Math.max(0, -0), 0)
    arktest.assertEQ(Math.max(-0, 0), 0)
    arktest.assertEQ(Math.max(-0, -0), -0)
}

function testWithFloats() {
    arktest.assertEQ(Math.max(1.5, 2.5), 2.5)
    arktest.assertEQ(Math.max(2.5, 1.5), 2.5)
    arktest.assertEQ(Math.max(1.1, 1.2, 1.3), 1.3)
    arktest.assertEQ(Math.max(-1.5, -2.5), -1.5)
    arktest.assertEQ(Math.max(0.1, 0.2, 0.3), 0.3)
}

function testNegativeAndPositive() {
    arktest.assertEQ(Math.max(-5, 5), 5)
    arktest.assertEQ(Math.max(5, -5), 5)
    arktest.assertEQ(Math.max(-10, 0, 10), 10)
    arktest.assertEQ(Math.max(10, 0, -10), 10)
}

function testLargeNumbers() {
    arktest.assertEQ(Math.max(1e100, 1e101), 1e101)
    arktest.assertEQ(Math.max(Number.MAX_VALUE, Number.MAX_VALUE - 1), Number.MAX_VALUE)
    arktest.assertEQ(Math.max(-Number.MAX_VALUE, -Number.MAX_VALUE + 1), -Number.MAX_VALUE + 1)
}

function testSmallNumbers() {
    arktest.assertEQ(Math.max(1e-100, 1e-101), 1e-100)
    arktest.assertEQ(Math.max(Number.MIN_VALUE, Number.MIN_VALUE * 2), Number.MIN_VALUE * 2)
}

function testMixedValues() {
    arktest.assertEQ(Math.max(100, -50, 0, 25, -75), 100)
    arktest.assertEQ(Math.max(1.5, -2.7, 0.5, -0.5), 1.5)
    arktest.assertEQ(Math.max(1000, 1, 100, 10), 1000)
}

function testCharOverload() {
    let a: char = c'a'
    let b: char = c'z'
    let c: char = c'A'
    let d: char = c'Z'

    arktest.assertEQ(maxChar(a, b), b)
    arktest.assertEQ(maxChar(b, a), b)
    arktest.assertEQ(maxChar(c, d), d)
    arktest.assertEQ(maxChar(a, c), a)  // 'a' > 'A' in ASCII
    arktest.assertEQ(maxChar(c'0', c'9'), c'9')
}

function testIntOverload() {
    let a: int = 10
    let b: int = 20
    let c: int = -5
    let d: int = 0

    arktest.assertEQ(Math.max(a, b), b)
    arktest.assertEQ(Math.max(b, a), b)
    arktest.assertEQ(Math.max(c, d), d)
    arktest.assertEQ(Math.max(a, c), a)
    arktest.assertEQ(Math.max(Int.MAX_VALUE, Int.MIN_VALUE), Int.MAX_VALUE)
}

function testLongOverload() {
    let a: long = 100000000000
    let b: long = 200000000000
    let c: long = -50000000000
    let d: long = 0

    arktest.assertEQ(Math.max(a, b), b)
    arktest.assertEQ(Math.max(b, a), b)
    arktest.assertEQ(Math.max(c, d), d)
    arktest.assertEQ(Math.max(a, c), a)
    arktest.assertEQ(Math.max(Long.MAX_VALUE, Long.MIN_VALUE), Long.MAX_VALUE)
}

function testFloatOverload() {
    let a: float = 1.5f
    let b: float = 2.5f
    let c: float = -1.5f
    let d: float = 0.0f

    arktest.assertEQ(Math.max(a, b), b)
    arktest.assertEQ(Math.max(b, a), b)
    arktest.assertEQ(Math.max(c, d), d)
    arktest.assertEQ(Math.max(a, c), a)
}

function testDoubleOverload() {
    let a: double = 1.5
    let b: double = 2.5
    let c: double = -1.5
    let d: double = 0.0

    arktest.assertEQ(Math.max(a, b), b)
    arktest.assertEQ(Math.max(b, a), b)
    arktest.assertEQ(Math.max(c, d), d)
    arktest.assertEQ(Math.max(a, c), a)
}

function main() {
    const suite = new arktest.ArkTestsuite("Math.max tests")
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
