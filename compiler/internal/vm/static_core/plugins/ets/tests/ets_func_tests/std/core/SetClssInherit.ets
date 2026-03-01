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


function main(): int {
    const suite = new arktest.ArkTestsuite("Set API test for SDK");
    suite.addTest("Inheritance testing of the Set class", testSetInherit);

    return suite.run();
}

class A<T> extends Set<T> {
    private newElement: number = 123;

    getElem(): number {
        return this.newElement;
    }

    setElem(elem: number): void {
        this.newElement = elem;
    }

    override toString(): String {
        return "Override toString() fun";
    }

    newFunc(): String {
        return "This is a new func";
    }
}

class TestData {
    static elem1 = 1;
    static elem2 = 2;
    static elem3 = 3;
    static expected1 = true;
    static expected2 = "Override toString() fun";;
    static expected3 = "This is a new func";

}

function testSetInherit() {
    let a: A<number> = new A<number>;
    a.add(TestData.elem1);
    a.add(TestData.elem2);
    a.add(TestData.elem3);

    let actual1 = a.has(2);
    let expected1 = TestData.expected1;
    arktest.assertEQ(actual1, expected1, "failed: Expected" + actual1 + "got: " + expected1);

    let actual2 = a.toString();
    let expected2 = TestData.expected2;
    arktest.assertEQ(actual2, expected2, "failed: Expected" + actual2 + "got: " + expected2);

    let actual3 = a.newFunc();
    let expected3 = TestData.expected3;
    arktest.assertEQ(actual3, expected3, "failed: Expected" + actual3 + "got: " + expected3);
}