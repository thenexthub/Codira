/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
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

const etalon: long[] = [
3609898888,
2658912113,
2687541264,
2596904264,
2744799565,
2773428716,
964447980,
2830687017,
2859316168,
3626958991,
3517786640,
3546415791,
3452210824,
3603674092,
3632303243,
1819754539,
3689561544,
3718190695,
187298255,
3775448996
]

function testString() {
    const start: double = 42.0
    let num: double = start
    const delta: double = 0.000000001
    const str = "str998"
    const qty: int = 5000

    let map = new Map<string, int>();
    for (let i: int = 0; i < qty; ++i) {
        num += delta
        const key = str + num
        map.set(key, i)
        if (i < etalon.length) {
            const hash = Runtime.getHashCodeByValue(key)
            arktest.assertEQ(hash, etalon[i].toInt())
        }
    }
}

function testGetHashCode1() {
  const test1= new Int(-1)
  arktest.assertEQ(Runtime.getHashCodeByValue(test1), -1);
}

function testGetHashCode2() {
  const test2 =  new Int(1)
  arktest.assertEQ(Runtime.getHashCodeByValue(test2), 1);
}

function testGetHashCode3() {
  const test3 = new Long(-1000000000)
  arktest.assertEQ(Runtime.getHashCodeByValue(test3), -1000000000);
}

function testGetHashCode4() {
  const test4 = new Long(1000000000)
  arktest.assertEQ(Runtime.getHashCodeByValue(test4), 1000000000);
}

function testGetHashCode5() {
  const test5 = new Byte(-128)
  arktest.assertEQ(Runtime.getHashCodeByValue(test5), -128);
}

function testGetHashCode6() {
  const test6 = new Byte(127)
  arktest.assertEQ(Runtime.getHashCodeByValue(test6), 127);
}

function testGetHashCode7() {
  const test7 = new Short(-32768)
  arktest.assertEQ(Runtime.getHashCodeByValue(test7), -32768);
}

function testGetHashCode8() {
  const test8 = new Short(32767)
  arktest.assertEQ(Runtime.getHashCodeByValue(test8), 32767);
}

function testGetHashCode9() {
    const test9 = new Double(Double.MAX_VALUE);
    arktest.assertEQ(Runtime.getHashCodeByValue(test9), Int.MAX_VALUE);
}
  
function testGetHashCode10() {
    const test10 = new Double(-Double.MAX_VALUE);
    arktest.assertEQ(Runtime.getHashCodeByValue(test10), Int.MIN_VALUE);
}

function testGetHashCode11() {
    const test11 = new Double(Double.NaN);
    arktest.assertEQ(Runtime.getHashCodeByValue(test11), 0);
}

function testGetHashCode12() {
    const test12 = new Double(Double.POSITIVE_INFINITY);
    arktest.assertEQ(Runtime.getHashCodeByValue(test12), Int.MAX_VALUE);
}

function testGetHashCode13() {
    const test13 = new Double(Double.NEGATIVE_INFINITY);
    arktest.assertEQ(Runtime.getHashCodeByValue(test13), Int.MIN_VALUE);
}

function testGetHashCode14() {
    const test14 = new Float(Float.MAX_VALUE);
    arktest.assertEQ(Runtime.getHashCodeByValue(test14), Int.MAX_VALUE);
}

function testGetHashCode15() {
    const test15 = new Float(-Float.MAX_VALUE);
    arktest.assertEQ(Runtime.getHashCodeByValue(test15), Int.MIN_VALUE);
}

function testGetHashCode16() {
    const test16 = new Float(Float.NaN);
    arktest.assertEQ(Runtime.getHashCodeByValue(test16), 0);
}

function testGetHashCode17() {
    const test17 = new Float(Float.POSITIVE_INFINITY);
    arktest.assertEQ(Runtime.getHashCodeByValue(test17), Int.MAX_VALUE);
}

function testGetHashCode18() {
    const test18 = new Float(Float.NEGATIVE_INFINITY);
    arktest.assertEQ(Runtime.getHashCodeByValue(test18), Int.MIN_VALUE);
}

function testGetHashCode19() {
    const test19 = new BigInt("12345678901234567890");
    arktest.assertEQ(Runtime.getHashCodeByValue(test19), 605486267);
}

function testGetHashCode20() {
    const test20 = new BigInt("-12345678901234567890");
    arktest.assertEQ(Runtime.getHashCodeByValue(test20), 605484345);
}

class A{
    b = new String("1")
}
function testGetHashCode21() {
    const key = new A()
    const hash = Runtime.getHashCodeByValue(key)
    arktest.assertLE(Int.MIN_VALUE, hash)
    arktest.assertLE(hash, Int.MAX_VALUE)
}

function testLong(): void {
  let vals = new Array<Long>(-10012345111, -298765, -12, -1, 0, 1, 16, 100, 12345, 9876543111)
  let hashCoderArr = new Array<Long>(vals.length);
  
  for (let i = 0; i < vals.length; i++) {
    hashCoderArr[i] = Runtime.getHashCodeByValue(vals[i]);
    arktest.assertEQ(hashCoderArr[i], vals[i].toInt());
  }
}

enum Status {
  PENDING,
  PROCESSING,
  COMPLETED
}
function testGetHashEnum01() {
  const set: Set<Status | number> = new Set<Status | number>([
      Status.PENDING,
      0, // Numeric equivalent of PENDING
      Status.PROCESSING,
      1, // Numeric equivalent of PROCESSING
      Status.COMPLETED
  ]);

  const values: (Status | number)[] = Array.from(set.values());
  const arr = new Array<int>(0, 1, 2)
  arktest.assertEQ(values.length, 3)
  arktest.assertEQ(values.toString(), arr.toString())
}

function testGetHashEnum02() {
  const map: Map<Status | number, string> = new Map<Status | number, string>([
      [Status.PENDING, "1"],
      [0, "1"],// Numeric equivalent of PENDING
      [Status.PROCESSING, "1"],
      [1, "1"],// Numeric equivalent of PROCESSING
      [Status.COMPLETED, "1"]
  ]);
  const values = Array.from(map.keys());
  const arr = new Array<int>(0, 1, 2)
  arktest.assertEQ(values.length, 3)
  arktest.assertEQ(values.toString(), arr.toString())
}

enum StatusString {
  PENDING = "pending",
  PROCESSING = "processing",
  COMPLETED = "completed"
}

function testGetHashEnumString01() {
  const set: Set<StatusString | string> = new Set<StatusString | string>([
    StatusString.PENDING,
    "pending", // String equivalent of PENDING
    StatusString.PROCESSING,
    "processing", // String equivalent of PROCESSING
    StatusString.COMPLETED
  ]);

  const values: (StatusString | string)[] = Array.from(set.values());
  const arr = new Array<string>("pending", "processing", "completed")
  arktest.assertEQ(values.length, 3)
  arktest.assertEQ(values.toString(), arr.toString())
}

function testGetHashEnumString02() {
  const map: Map<StatusString | string, string> = new Map<StatusString | string, string>([
    [StatusString.PENDING, "1"],
    ["pending", "1"], // String equivalent of PENDING
    [StatusString.PROCESSING, "1"],
    ["processing", "1"], // String equivalent of PROCESSING
    [StatusString.COMPLETED, "1"]
  ]);
  const values = Array.from(map.keys());
  const arr = new Array<string>("pending", "processing", "completed")
  arktest.assertEQ(values.length, 3)
  arktest.assertEQ(values.toString(), arr.toString())
}

////////////////////////////////////////////////////////////////

function main(): int {
    const suite = new arktest.ArkTestsuite("GenerateGetHashCodeByValue tests")

    suite.addTest("Get hash code of string", testString)
    suite.addTest("Get hash code of int", testGetHashCode1)
    suite.addTest("Get hash code of int", testGetHashCode2)
    suite.addTest("Get hash code of long", testGetHashCode3)
    suite.addTest("Get hash code of long", testGetHashCode4)
    suite.addTest("Get hash code of byte", testGetHashCode5)
    suite.addTest("Get hash code of byte", testGetHashCode6)
    suite.addTest("Get hash code of short", testGetHashCode7)
    suite.addTest("Get hash code of short", testGetHashCode8)
    suite.addTest("Get hash code of double", testGetHashCode9)
    suite.addTest("Get hash code of double", testGetHashCode10)
    suite.addTest("Get hash code of double", testGetHashCode11)
    suite.addTest("Get hash code of double", testGetHashCode12)
    suite.addTest("Get hash code of double", testGetHashCode13)
    suite.addTest("Get hash code of float", testGetHashCode14)
    suite.addTest("Get hash code of float", testGetHashCode15)
    suite.addTest("Get hash code of float", testGetHashCode16)
    suite.addTest("Get hash code of float", testGetHashCode17)
    suite.addTest("Get hash code of float", testGetHashCode18)
    suite.addTest("Get hash code of bigint", testGetHashCode19)
    suite.addTest("Get hash code of bigint", testGetHashCode20)
    suite.addTest("Get hash code of object", testGetHashCode21)
    suite.addTest("Get hash code of testLong", testLong)
    suite.addTest("Get hash code of enum", testGetHashEnum01)
    suite.addTest("Get hash code of enum", testGetHashEnum02)
    suite.addTest("Get hash code of enum string", testGetHashEnumString01)
    suite.addTest("Get hash code of enum string", testGetHashEnumString02)

    return suite.run()
}