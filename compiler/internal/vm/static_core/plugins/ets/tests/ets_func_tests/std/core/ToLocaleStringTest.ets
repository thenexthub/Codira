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

function testArrayToLocaleString1(): void {
    const array = ['￥7', 500, 8123, 12];
    let value = array.toLocaleString();
    arktest.assertEQ(value, '￥7,500,8,123,12')
}
function testArrayToLocaleString2(): void {
    const array = ['$10', 500, 823, 1234];
    let value = array.toLocaleString();
    arktest.assertEQ(value, '$10,500,823,1,234')
}
function testArrayToLocaleString3(): void {
    const array = [new Short(100), new Char(c'B'), new Byte(50)];
    let value = array.toLocaleString();
    arktest.assertEQ(value, '100,B,50')
}
function testArrayToLocaleString4(): void {
    const array = [new Long(1000000), new Short(25), new Char(c'X')];
    let value = array.toLocaleString();
    arktest.assertEQ(value, '1,000,000,25,X')
}
function testArrayToLocaleString5(): void {
    const array = [new Byte(10), new Long(1000000000), new Float(3.14)];
    let value = array.toLocaleString();
    arktest.assertEQ(value, '10,1,000,000,000,3.14')
}
function testNaNToLocaleString(): void {
    const num = NaN;
    let value = num.toLocaleString();
    arktest.assertEQ(value, 'NaN')
}
function testInfinityToLocaleString(): void {
    const num = Infinity;
    let value = num.toLocaleString();
    arktest.assertEQ(value, '∞')
}
function testNegativeToLocaleString(): void {
    const num = new Int(-1234);
    let value = num.toLocaleString();
    arktest.assertEQ(value, '-1,234')
}
function testbyteToLocaleString(): void {
    const num = new Byte(10)
    let value = num.toLocaleString();
    arktest.assertEQ(value, '10')
}
function testshortToLocaleString(): void {
    const num = new Short(10)
    let value = num.toLocaleString();
    arktest.assertEQ(value, '10')
}
function testintToLocaleString(): void {
    const num = new Int(1000)
    let value = num.toLocaleString();
    arktest.assertEQ(value, '1,000')
}
function testlongToLocaleString(): void {
    const num = new Long(100000000000)
    let value = num.toLocaleString();
    arktest.assertEQ(value, '100,000,000,000')
}
function testfloatToLocaleString(): void {
    const num = new Float(1000.1)
    let value = num.toLocaleString();
    arktest.assertEQ(value, '1,000.1')
}
function testcharToLocaleString(): void {
    const num = new Char(c'a')
    let value = num.toLocaleString();
    arktest.assertEQ(value, 'a')
}

function main(): int {
    const suite = new arktest.ArkTestsuite("ToLocaleString")
    suite.addTest("toLocaleString1", testArrayToLocaleString1)
    suite.addTest("toLocaleString2", testArrayToLocaleString2)
    suite.addTest("toLocaleString3", testArrayToLocaleString3)
    suite.addTest("toLocaleString4", testArrayToLocaleString4)
    suite.addTest("toLocaleString5", testArrayToLocaleString5)
    suite.addTest("NaNToLocaleString", testNaNToLocaleString)
    suite.addTest("InfinityToLocaleString", testInfinityToLocaleString)
    suite.addTest("negativeToLocaleString", testNegativeToLocaleString)
    suite.addTest("byteToLocaleString", testbyteToLocaleString)
    suite.addTest("shortToLocaleString", testshortToLocaleString)
    suite.addTest("intToLocaleString", testintToLocaleString)
    suite.addTest("longToLocaleString", testlongToLocaleString)
    suite.addTest("floatToLocaleString", testfloatToLocaleString)
    suite.addTest("charToLocaleString", testcharToLocaleString)
    return suite.run()
}
