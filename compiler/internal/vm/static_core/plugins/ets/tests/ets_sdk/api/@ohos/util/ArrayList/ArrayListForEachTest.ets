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

function main() {
    const suite = new arktest.ArkTestsuite("ArrayList API tests");

    suite.addTest("Testing ArrayList replaceAllElement add same data", testArrayListReplaceAllElement0001);
    suite.addTest("Testing ArrayList replaceAllElement insert new string", testArrayListReplaceAllElement0002);
    suite.addTest("Testing ArrayList forEach", testArrayListForEach0001);
    suite.addTest("Testing ArrayList forEach->convertToArray", testArrayListForEach0002);
    suite.addTest("Testing ArrayList add->forEach->convertToArray", testArrayListForEach0003);
    suite.addTest("Testing ArrayList replaceAllElements with + 1 ->convertToArray", testArrayListReplaceAllElements0001);
    suite.addTest("Testing ArrayList replaceAllElements with - 1 ->convertToArray", testArrayListReplaceAllElements0002);
    suite.addTest("Testing ArrayList replaceAllElements with another arrayList", testArrayListReplaceAllElements0003);
    suite.addTest("Testing ArrayList replaceAllElements with another empty arrayList",
        testArrayListReplaceAllElements0004);

    return suite.run();
}

function testArrayListReplaceAllElement0001() {
    let arrayList = new ArrayList<string>();
    arrayList.add("zhang san");
    arrayList.add("");
    arrayList.add("`");
    arrayList.add("%");
    arrayList.add(`"`);
    arrayList.replaceAllElements((value: string, index: int) => {
        let tmp = value;
        value = tmp + value;
        let result = arrayList[index];
        arktest.assertEQ(value, result + result, `The value at index ${index} should be '${result + result}'`);
        return value;
    });
}

function testArrayListReplaceAllElement0002() {
    let arrayList = new ArrayList<double | string | Array<double | string | boolean> | boolean | undefined | null |
    Object>();
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
    arrayList.replaceAllElements((value, index, tempList) => {
        let length = tempList ? tempList.length - 1 : 0;
        if (index === length) {
            let tmp = value as string;
            value = tmp + "add last index";
        }
        return value;
    });
    arktest.assertTrue(arrayList.has(`"add last index`),
        "The ArrayList should contain the modified last element with 'add last index'");
}

function testArrayListForEach0001() {
    let arrayList = new ArrayList<double | string | Array<double | string | boolean> | boolean | undefined | null |
    Object>();
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
    arrayList.forEach((value) => {
        arktest.assertTrue(arrayList.has(value), `The ArrayList should contain the value '${value}'`);
    });
}

function testArrayListForEach0002() {
    let arrayList = new ArrayList<double>();
    let newArrayList = new ArrayList<double>();
    let result = new ArrayList<double>();
    arrayList.add(2);
    arrayList.add(4);
    arrayList.add(5);
    arrayList.add(4);
    arrayList.forEach((value: double, index: int) => {
        result.add(value);
        result.add(index);
    });

    let expectedValuesForArrayList = [2, 4, 5, 4];
    let expectedValuesForResult = [2, 0, 4, 1, 5, 2, 4, 3];
    for (let i: int = 0; i < arrayList.length; i++) {
        arktest.assertEQ(arrayList[i], expectedValuesForArrayList[i],
            `The element at index ${i} should be ${expectedValuesForArrayList[i]}`);
    }
    for (let i: int = 0; i < result.length; i++) {
        arktest.assertEQ(result[i], expectedValuesForResult[i],
            `The element at index ${i} should be ${expectedValuesForResult[i]}`);
    }
    arktest.assertEQ(newArrayList.length, 0, "The length of newArrayList should be 0 without adding elements");
}

function testArrayListForEach0003() {
    let arrayList = new ArrayList<double>();
    let newArrayList = new ArrayList<double>();
    let result = new ArrayList<double>();
    arrayList.add(2);
    arrayList.add(4);
    arrayList.add(5);
    arrayList.add(4);
    newArrayList.add(4);
    arrayList.forEach((value: double, index: int) => {
        result.add(value);
        result.add(index);
    });

    let expectedValuesForArrayList = [2, 4, 5, 4];
    let expectedValuesForResult = [2, 0, 4, 1, 5, 2, 4, 3];
    for (let i: int = 0; i < arrayList.length; i++) {
        arktest.assertEQ(arrayList[i], expectedValuesForArrayList[i],
            `The element at index ${i} should be ${expectedValuesForArrayList[i]}`);
    }
    for (let i: int = 0; i < result.length; i++) {
        arktest.assertEQ(result[i], expectedValuesForResult[i],
            `The element at index ${i} should be ${expectedValuesForResult[i]}`);
    }
    arktest.assertEQ(newArrayList.length, 1, "The length of newArrayList should be 1 after adding an element");

}

function testArrayListReplaceAllElements0001() {
    let arrayList = new ArrayList<double>();
    arrayList.add(2);
    arrayList.add(4);
    arrayList.add(5);
    arrayList.add(4);
    arrayList.replaceAllElements((value: double): double => {
        return value + 1;
    });

    let expectedValues = [3, 5, 6, 5];
    for (let i: int = 0; i < arrayList.length; i++) {
        arktest.assertEQ(arrayList[i], expectedValues[i],
            `The element at index ${i} should be ${expectedValues[i]}`);
    }
}

function testArrayListReplaceAllElements0002() {
    let arrayList = new ArrayList<double>();
    arrayList.add(2);
    arrayList.add(4);
    arrayList.add(5);
    arrayList.add(4);
    arrayList.replaceAllElements((value: double): double => {
        return value - 1;
    });

    let expectedValues = [1, 3, 4, 3];
    for (let i: int = 0; i < arrayList.length; i++) {
        arktest.assertEQ(arrayList[i], expectedValues[i],
            `The element at index ${i} should be ${expectedValues[i]}`);
    }
}

function testArrayListReplaceAllElements0003() {
    let arrayList = new ArrayList<double>();
    let newArrayList = new ArrayList<double>();
    arrayList.add(2);
    arrayList.add(4);
    arrayList.add(5);
    arrayList.add(4);
    arrayList.replaceAllElements((value: double): double => {
        return value - 1;
    });

    let expectedValues = [1, 3, 4, 3];
    for (let i: int = 0; i < arrayList.length; i++) {
        arktest.assertEQ(arrayList[i], expectedValues[i],
            `The element at index ${i} should be ${expectedValues[i]}`);
    }
    arktest.assertEQ(newArrayList.length, 0, "The length of newArrayList should be 0 without adding elements");
}

function testArrayListReplaceAllElements0004() {
    let arrayList = new ArrayList<double>();
    let newArrayList = new ArrayList<double>();
    arrayList.add(2);
    arrayList.add(4);
    arrayList.add(5);
    arrayList.add(4);
    newArrayList.add(3);
    newArrayList.add(5);
    newArrayList.add(7);
    arrayList.replaceAllElements((value: double): double => {
        return value - 1;
    });

    let expectedValuesForArrayList = [1, 3, 4, 3];
    let expectedValuesForNew = [3, 5, 7];
    for (let i: int = 0; i < arrayList.length; i++) {
        arktest.assertEQ(arrayList[i], expectedValuesForArrayList[i],
            `The element at index ${i} should be ${expectedValuesForArrayList[i]}`);
    }
    for (let i: int = 0; i < newArrayList.length; i++) {
        arktest.assertEQ(newArrayList[i], expectedValuesForNew[i],
            `The element at index ${i} should be ${expectedValuesForNew[i]}`);
    }
}
