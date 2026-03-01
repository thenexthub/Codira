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


function testLongMax(): void {
    const a : long = Long.MAX_VALUE
    arktest.assertEQ(a.toLocaleString(), "9,223,372,036,854,775,807")
}

function testLongMin(): void {
    const b : long = Long.MIN_VALUE
    arktest.assertEQ(b.toLocaleString(), "-9,223,372,036,854,775,808")
}

function testLong1(): void {
    const a : long = 3
    arktest.assertEQ(a.toLocaleString(), "3")
}

function testLong2(): void {
    const b : long = 1000
    arktest.assertEQ(b.toLocaleString(), "1,000")
}

function testLong3(): void {
    const c : long = 1000000
    arktest.assertEQ(c.toLocaleString(), "1,000,000")
}

function testFormatLong1() :void{
    const a = new Long(1234567890);
    const value = new Intl.NumberFormat().format(a)
    arktest.assertEQ(value, "1,234,567,890")
}

function testFormatLong2() :void{
    const b = new Long(-1234567890);
    const value = new Intl.NumberFormat().format(b)
    arktest.assertEQ(value, "-1,234,567,890")
}
function foo(): number | Long{
    return (12345689234).toLong()
}

function testFormatLong3(): void {
    let a : number | long = foo()
    const value = new Intl.NumberFormat().format(a)
    arktest.assertEQ(value, "12,345,689,234")
}

function main() {
    let myTestsuite = new arktest.ArkTestsuite("ToLocaleStringLong")
    myTestsuite.addTest("testLongToLocaleString1", testLongMax)
    myTestsuite.addTest("testLongToLocaleString2", testLongMin)
    myTestsuite.addTest("testLongToLocaleString3", testLong1)
    myTestsuite.addTest("testLongToLocaleString4", testLong2)
    myTestsuite.addTest("testLongToLocaleString5", testLong3)
    myTestsuite.addTest("testLongToLocaleString6", testFormatLong1)
    myTestsuite.addTest("testLongToLocaleString7", testFormatLong2)
    myTestsuite.addTest("testLongToLocaleString8", testFormatLong3)
    return myTestsuite.run()
}