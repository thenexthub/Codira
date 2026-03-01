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

import {util} from "@ohos.util";
import { BusinessError } from "@ohos.base";

interface Person {
    name: string,
    age: number
}

function main(): int {
    const suite = new arktest.ArkTestsuite("CommonUtil getHash API tests")

    suite.addTest("Get the hash value of an Interface object", testUtilGetHash001)
    suite.addTest("Get the hash value of an array of numbers", testUtilGetHash002)
    suite.addTest("Get the hash value of a Class object", testUtilGetHash003)
    suite.addTest("Get the hash value of a Date object", testUtilGetHash004)
    suite.addTest("Get the hash value of a RegExp object", testUtilGetHash005)
    suite.addTest("Get the hash value of a Math object", testUtilGetHash006)
    suite.addTest("Get the hash value of a Math object and a string", testUtilGetHash007)
    suite.addTest("Get the hash value of two Class object", testUtilGetHash008)
    suite.addTest("Get the hash value of a number data and a string", testUtilGetHash009)
    suite.addTest("Get the hash value of same Class object", testUtilGetHash010)
    suite.addTest("Get the hash value of same RegExp object", testUtilGetHash011)

    return suite.run()
}

function testUtilGetHash001() {
    let person: Person = {
        name: "Alice",
        age: 30
    };
    let result1 = util.getHash(person);
    let result2 = util.getHash(person);
    arktest.assertEQ(result1, result2, "The same object should have the same hash code");
}

function testUtilGetHash002() {
    let numbers = [1, 2, 3, 4, 5];
    let result1 = util.getHash(numbers);
    let result2 = util.getHash(numbers);
    arktest.assertEQ(result1, result2, "The same object should have the same hash code");
}

class Car {
    make: string;
    model: string;

    constructor(make: string, model: string) {
        this.make = make;
        this.model = model;
    }
}
function testUtilGetHash003() {

    let myCar = new Car("Toyota", "Corolla");
    let result1 = util.getHash(myCar);
    let result2 = util.getHash(myCar);
    arktest.assertEQ(result1, result2, "The same object should have the same hash code");
}

function testUtilGetHash004() {
    let now = new Date();
    let result1 = util.getHash(now);
    let result2 = util.getHash(now);
    arktest.assertEQ(result1, result2, "The same object should have the same hash code");
}

function testUtilGetHash005() {
    let regex = new RegExp("abc");
    let result1 = util.getHash(regex);
    let result2 = util.getHash(regex);
    arktest.assertEQ(result1, result2, "The same object should have the same hash code");
}

function testUtilGetHash006() {
    let mathObject: Math = new Math();
    let result1 = util.getHash(mathObject);
    let result2 = util.getHash(mathObject);
    arktest.assertEQ(result1, result2, "The same object should have the same hash code");
}

function testUtilGetHash007() {
    let mathObject: Math = new Math();
    let test: string = "test";
    let result1 = util.getHash(mathObject);
    let result2 = util.getHash(test);
    arktest.assertTrue(result1 !== result2, "The different object should have the different hash code");
}

function testUtilGetHash008() {
    let person: Person = {
        name: "Alice",
        age: 30
    };
    let person1: Person = {
        name: "Lily",
        age: 25
    };
    let result1 = util.getHash(person);
    let result2 = util.getHash(person1);
    arktest.assertTrue(result1 !== result2, "The different object should have the different hash code");
}

function testUtilGetHash009() {
    let data: number = 11;
    let test: string = "test";
    let result1 = util.getHash(data);
    let result2 = util.getHash(test);
    arktest.assertTrue(result1 !== result2, "The different object should have the different hash code");
}

function testUtilGetHash010() {

    let myCar1 = new Car("Toyota", "Corolla");
    let myCar2 = new Car("Toyota", "Corolla");
    let result1 = util.getHash(myCar1);
    let result2 = util.getHash(myCar2);
    arktest.assertNE(result1, result2, "The object should have the different hash code");
}

function testUtilGetHash011() {
    let regex1 = new RegExp("abc");
    let regex2 = new RegExp("abc");
    let result1 = util.getHash(regex1);
    let result2 = util.getHash(regex2);
    arktest.assertNE(result1, result2, "The object should have the different hash code");
}

function testUtilGetHash012() {
    try {
        util.getHash(null);
    } catch (e) {
        e = e as BusinessError;
        arktest.assertEQ(e.message, "Parameter error. The type of Parameter must be object.");
        return;
    }
}

function testUtilGetHash013() {
    try {
        util.getHash(undefined);
    } catch (e) {
        e = e as BusinessError;
        arktest.assertEQ(e.message, "Parameter error. The type of Parameter must be object.");
        return;
    }
}
