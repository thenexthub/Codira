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

import Deque from "@ohos.util.Deque";

class C1 {
    name: string = "";
    age: string = "";
}

function main() {
    const suite = new arktest.ArkTestsuite("Deque API tests");

    suite.addTest("Test inserting string elements at the front and retrieving the first element.",
        testInsertStringAtFront);
    suite.addTest("Test inserting numeric elements at the front and retrieving the first element.",
        testInsertNumberAtFront);
    suite.addTest("Test inserting object elements at the front and retrieving the first element.",
        testInsertObjectAtFront);
    suite.addTest("Test inserting mixed-type elements at the front and retrieving the first element.",
        testInsertMixedTypesAtFront);
    suite.addTest("Test inserting string elements at the end and retrieving the last element.", testInsertStringAtEnd);
    suite.addTest("Test inserting numeric elements at the end and retrieving the last element.", testInsertNumberAtEnd);
    suite.addTest("Test inserting object elements at the end and retrieving the last element.", testInsertObjectAtEnd);
    suite.addTest("Test inserting mixed-type elements at the end and retrieving the last element.",
        testInsertMixedTypesAtEnd);

    return suite.run();
}

// Test cases ported from ArkTS 1.0: DEQUE_0100
function testInsertStringAtFront() {
    let deque = new Deque<string>();
    deque.insertFront("a");
    deque.insertFront("b");
    deque.insertFront("c");
    let result = deque.getFirst();
    arktest.assertEQ(result, "c");
}

// Test cases ported from ArkTS 1.0: DEQUE_0200
function testInsertNumberAtFront() {
    let deque = new Deque<double>();
    deque.insertFront(1);
    deque.insertFront(2);
    deque.insertFront(3);
    let result = deque.getFirst();
    arktest.assertEQ(result, 3);
}

// Test cases ported from ArkTS 1.0: DEQUE_0300
function testInsertObjectAtFront() {
    let deque = new Deque<object>();
    let a: C1 = {
        name: "Dylon", age: "13"
    };
    let b: C1 = {
        name: "Joe", age: "14"
    };
    let c: C1 = {
        name: "Lucy", age: "15"
    };
    deque.insertFront(a);
    deque.insertFront(b);
    deque.insertFront(c);
    let result = deque.getFirst();
    arktest.assertEQ(result, c);
}

// Test cases ported from ArkTS 1.0: DEQUE_0400
function testInsertMixedTypesAtFront() {
    let deque = new Deque<double | string | object>();
    let a: C1 = {
        name: "Dylon", age: "13"
    };
    let b = 'a';
    let c = 1;
    deque.insertFront(a);
    deque.insertFront(b);
    deque.insertFront(c);
    let result = deque.getFirst();
    arktest.assertEQ(result, c);
}

// Test cases ported from ArkTS 1.0: DEQUE_0500
function testInsertStringAtEnd() {
    let deque = new Deque<string>();
    deque.insertEnd("a");
    deque.insertEnd("b");
    deque.insertEnd("c");
    let result = deque.getLast();
    arktest.assertEQ(result, "c");
}

// Test cases ported from ArkTS 1.0: DEQUE_0600
function testInsertNumberAtEnd() {
    let deque = new Deque<double>();
    deque.insertEnd(1);
    deque.insertEnd(2);
    deque.insertEnd(3);
    let result = deque.getLast();
    arktest.assertEQ(result, 3);
}

// Test cases ported from ArkTS 1.0: DEQUE_0700
function testInsertObjectAtEnd() {
    let deque = new Deque<object>();
    let a: C1 = {
        name: "Dylon", age: "13"
    };
    let b: C1 = {
        name: "Joe", age: "14"
    };
    let c: C1 = {
        name: "Lucy", age: "15"
    };
    deque.insertEnd(a);
    deque.insertEnd(b);
    deque.insertEnd(c);
    let result = deque.getLast();
    arktest.assertEQ(result, c);
}

// Test cases ported from ArkTS 1.0: DEQUE_0800
function testInsertMixedTypesAtEnd() {
    let deque = new Deque<double | string | object>();
    let a: C1 = {
        name: "Dylon", age: "13"
    };
    let b = 'a';
    let c = 1;
    deque.insertEnd(a);
    deque.insertEnd(b);
    deque.insertEnd(c);
    let result = deque.getLast();
    arktest.assertEQ(result, c);
}
