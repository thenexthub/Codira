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

import {ArrayList} from "@ohos.util.ArrayList";

class Temp {
    age: double = 0;
    name: string = "";
}

class Dog {
    age: double = 0;
    name: string = "";
    constructor(age: double, name: string) {
        this.age = age;
        this.name = name;
    }
}

class Cat {
    age1: double = 0;
    name1: string = "";
    constructor(age: double, name: string) {
        this.age1 = age;
        this.name1 = name;
    }
}

function main() {
    const suite = new arktest.ArkTestsuite("ArrayList API tests");

    suite.addTest("Testing ArrayList insert->add->remove->subArrayList->getCapacity", testArrayListGetCapacity0001);
    suite.addTest("Testing ArrayList insert->add->remove->subArrayList->clear->getCapacity",
        testArrayListGetCapacity0002);
    suite.addTest("Testing ArrayList convertToArray filter", testArrayListConvertToArray0001);
    suite.addTest("Testing ArrayList convertToArray fill", testArrayListConvertToArray0002);
    suite.addTest("Testing ArrayList isEmpty", testArrayListIsEmpty0001);
    suite.addTest("Testing ArrayList isEmpty has data", testArrayListIsEmpty0002);
    suite.addTest("Testing ArrayList isEmpty clear->isEmpty", testArrayListIsEmpty0003);
    suite.addTest("Testing ArrayList symbol.iterator", testArrayListSymbolIterator0001);
    suite.addTest("Testing ArrayList for..of", testArrayListSymbolIterator0002);

    return suite.run();
}

function testArrayListGetCapacity0001() {
    let arrayList = new ArrayList<double | string>();
    arrayList.insert(2, 0);
    arrayList.add(1);
    arrayList.add(22);
    arrayList.remove(1);
    arrayList.increaseCapacityTo(1);
    arrayList.subArrayList(1, 2);
    arrayList.trimToCurrentLength();
    let result = arrayList.getCapacity();
    arktest.assertEQ(result, arrayList.length, "The capacity of the ArrayList should match its length after trimming");
}

function testArrayListGetCapacity0002() {
    let arrayList = new ArrayList<double | string>();
    arrayList.insert(2, 0);
    arrayList.add(1);
    arrayList.add(22);
    arrayList.remove(1);
    arrayList.subArrayList(1, 2);
    arrayList.clear();
    arrayList.trimToCurrentLength();
    arktest.assertEQ(arrayList.getCapacity(), 0, "The capacity of the ArrayList should be 0 after clearing and trimming");
    arrayList.increaseCapacityTo(1);
    arktest.assertEQ(arrayList.getCapacity(), 1, "The capacity of the ArrayList should be 1 after increasing capacity");
}

function testArrayListConvertToArray0001() {
    let arrayList = new ArrayList<Dog>();
    let dog = new Dog(1, "jiu jiu");
    arrayList.add(dog);
    let array = arrayList.convertToArray();
    array.filter((value) => {
        arktest.assertTrue(value instanceof Dog, "Each element in the array should be an instance of Dog");
        return (value instanceof Dog);
    });
}

function testArrayListConvertToArray0002() {
    let arrayList = new ArrayList<Dog | Cat>();
    let dog = new Dog(1, "jiu jiu");
    arrayList.add(dog);
    let array = arrayList.convertToArray();
    let cat = new Cat(2, "li li");
    array.fill(cat);
    arktest.assertFalse(array[0] instanceof Dog, "The first element in the array should no longer be an instance of Dog after filling with Cat");
}

function testArrayListIsEmpty0001() {
    let arrayList = new ArrayList<double>();
    arktest.assertTrue(arrayList.isEmpty(), "The ArrayList should be empty initially");
}

function testArrayListIsEmpty0002() {
    let arrayList = new ArrayList<double | string | Array<double | string | boolean> | boolean | undefined | null |
    Object>();
    addTestElements(arrayList);
    arktest.assertFalse(arrayList.isEmpty(), "The ArrayList should not be empty after adding elements");
}

function testArrayListIsEmpty0003() {
    let arrayList = new ArrayList<double | string | Array<double | string | boolean> | boolean | undefined | null |
    Object>();
    addTestElements(arrayList);
    arrayList.clear();
    arktest.assertTrue(arrayList.isEmpty(), "The ArrayList should be empty after clearing all elements");
}

function testArrayListSymbolIterator0001() {
    let arrayList = new ArrayList<double | string | Array<double | string | boolean> | boolean | undefined | null |
    Object>();
    addTestElements(arrayList);
    let iter = arrayList.$_iterator();
    let tem = iter.next();
    while (!tem.done) {
        arktest.assertTrue(arrayList.has(tem.value), `The ArrayList should contain the value '${tem.value}'`);
        tem = iter.next();
    }
}

function testArrayListSymbolIterator0002() {
    let arrayList = new ArrayList<double>();
    arrayList.add(2);
    arrayList.add(4);
    arrayList.add(5);
    arrayList.add(4);

    let expectedValues = [2, 4, 5, 4];
    let index = 0;
    for (let item of arrayList) {
        arktest.assertEQ(item, expectedValues[index], `The element at index ${index} should be ${expectedValues[index]}`);
        index++;
    }
}

function addTestElements(arrayList: ArrayList<double | string | Array<double | string | boolean> | boolean | undefined |
    null | Object>) {
    arrayList.add("zhang san");
    arrayList.add(1);
    let temp: Temp = {
        age: 10, name: "zhang"
    };
    arrayList.add(temp);
    arrayList.add(undefined);
    arrayList.add(null);
    arrayList.add(true);
    let array = [0, 1, 2, undefined];
    arrayList.add(array);
    arrayList.add("");
    arrayList.add("`");
    arrayList.add("%");
    arrayList.add(`"`);
}
