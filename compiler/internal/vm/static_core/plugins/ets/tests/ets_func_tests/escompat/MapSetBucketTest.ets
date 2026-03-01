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

class TestClassA {
}

class TestClassB {
    b1: number
    b2: string
    b3: TestClassA = new TestClassA()
    b4: boolean
    b5: int
    constructor(b: int) {
        this.b1 = b + 1.1
        this.b2 = "abc" + b
        this.b4 = (b === 0)
        this.b5 = b
    }
}

function testSetInitialBuckets(): void {
    let t1 = new TestClassB(1)
    let t2 = new TestClassB(2)
    let t3 = new TestClassB(3)
    let set1 = new Set<TestClassB>(16)
    set1.add(t1)
    set1.add(t2)
    set1.add(t3)
    arktest.assertTrue(set1.size === 3)
    arktest.assertTrue(set1.has(t1) === true)
    arktest.assertTrue(set1.has(t2) === true)
    arktest.assertTrue(set1.has(t3) === true)
}

function testMapInitialBuckets(): void {
    let map1: Map<string, TestClassB> = new Map<string, TestClassB>(16)
    let m1 = new TestClassB(1)
    let m2 = new TestClassB(2)
    let m3 = new TestClassB(3)
    map1.set("m1", m1)
    map1.set("m2", m2)
    map1.set("m3", m3)
    arktest.assertTrue(map1.size === 3)
    arktest.assertTrue(map1.get("m1") === m1)
    arktest.assertTrue(map1.get("m2") === m2)
    arktest.assertTrue(map1.get("m3") === m3)
}

function main(): int {
    const suite = new arktest.ArkTestsuite("Map Set Bucket")
    suite.addTest("Set with initial Buckets", testSetInitialBuckets)
    suite.addTest("Map with initial Buckets", testMapInitialBuckets)
    return suite.run()
}