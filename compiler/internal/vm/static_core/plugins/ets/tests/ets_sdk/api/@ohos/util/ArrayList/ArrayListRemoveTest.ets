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
import {BusinessError} from "@ohos.base";

const OutOfBoundsErrorCodeId: int = 10200001;

class Temp {
    age: double = 0;
    name: string = "";
}

function main() {
    const suite = new arktest.ArkTestsuite("ArrayList API tests");

    suite.addTest("Testing ArrayList removeByIndex", testArrayListRemoveByIndexOf0001);
    suite.addTest("Testing ArrayList removeByIndex -2", testArrayListRemoveByIndexOf0002);
    suite.addTest("Testing ArrayList removeByIndex index > arrayList.length -1", testArrayListRemoveByIndexOf0003);
    suite.addTest("Testing ArrayList remove same data and check", testArrayListRemove0001);
    suite.addTest("Testing ArrayList removeByIndex same data and check", testArrayListRemove0002);
    suite.addTest("Testing ArrayList removeByRange fromIndex == toIndex", testArrayListRemoveByRange0001);
    suite.addTest("Testing ArrayList removeByRange toIndex is arrayList.length", testArrayListRemoveByRange0002);

    return suite.run();
}

function testArrayListRemoveByIndexOf0001() {
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
    let index1 = arrayList.getIndexOf("");
    let result = arrayList.removeByIndex(index1);
    let result1 = arrayList.removeByIndex(index1);
    arktest.assertNE(result, result1, "The results of consecutive removeByIndex calls at the same index should not be equal");
    let index2 = arrayList.getIndexOf("%");
    let result2 = arrayList.removeByIndex(index2);
    let result3 = arrayList.removeByIndex(index2);
    arktest.assertNE(result2, result3, "The results of consecutive removeByIndex calls at the same index should not be equal");
}

function testArrayListRemoveByIndexOf0002() {
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
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.code == OutOfBoundsErrorCodeId && e.message ==
                "The value of \"index\" is out of range. It must be >= 0 && <= 10. Received value is: -2";
        }
        return false;
    }
    arktest.expectThrow(() => {arrayList.removeByIndex(-2)}, exceptionCheck);
    let exceptionCheck1 = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.code == OutOfBoundsErrorCodeId && e.message ==
                "The value of \"index\" is out of range. It must be >= 0 && <= 10. Received value is: 11";
        }
        return false;
    }
    arktest.expectThrow(() => {arrayList.removeByIndex(arrayList.length)}, exceptionCheck1);
}

function testArrayListRemoveByIndexOf0003() {
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
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.code == OutOfBoundsErrorCodeId && e.message ==
                "The value of \"index\" is out of range. It must be >= 0 && <= 10. Received value is: 11";
        }
        return false;
    }
     arktest.expectThrow(() => {arrayList.removeByIndex(arrayList.length)}, exceptionCheck);
}

function testArrayListRemove0001() {
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
    arrayList.remove("zhang san");
    arrayList.remove(1);
    arrayList.remove(temp);
    arrayList.remove(undefined);
    arrayList.remove(null);
    arrayList.remove(true);
    arrayList.remove(array);
    arrayList.remove("");
    arrayList.remove("`");
    arrayList.remove("%");
    arrayList.remove(`"`);
    arrayList.insert("zhang san", 0);
    arrayList.insert("zhang san", 1);
    arrayList.insert("zhang san", 2);
    let index = arrayList.getIndexOf("zhang san");
    arktest.assertEQ(index, 0, "The index of 'zhang san' should be 0 after insertion");
    arrayList.remove("zhang san");
    let index1 = arrayList.getIndexOf("zhang san");
    arktest.assertEQ(index1, 0, "The index of 'zhang san' should still be 0 after removing one instance");
}

function testArrayListRemove0002() {
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
    arrayList.remove("zhang san");
    arrayList.remove(1);
    arrayList.remove(temp);
    arrayList.remove(undefined);
    arrayList.remove(null);
    arrayList.remove(true);
    arrayList.remove(array);
    arrayList.remove("");
    arrayList.remove("`");
    arrayList.remove("%");
    arrayList.remove(`"`);
    arrayList.insert("zhang san", 0);
    arrayList.insert("zhang san", 1);
    arrayList.insert("zhang san", 2);
    let index = arrayList.getIndexOf("zhang san");
    arktest.assertEQ(index, 0, "The index of 'zhang san' should be 0 after insertion");
    arrayList.removeByIndex(index);
    let index1 = arrayList.getIndexOf("zhang san");
    arktest.assertEQ(index1, 0, "The index of 'zhang san' should still be 0 after removing one instance by index");
}

function testArrayListRemoveByRange0001() {
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
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.code == OutOfBoundsErrorCodeId && e.message ==
                "The value of \"fromIndex\" is out of range. It must be >= 0 && <= 0. Received value is: 1";
        }
        return false;
    }
    arktest.expectThrow(() => {arrayList.removeByRange(1, 1)}, exceptionCheck);
}

function testArrayListRemoveByRange0002() {
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
    arrayList.removeByRange(1, arrayList.length);
    arktest.assertEQ(arrayList.length, 1, `The length of the ArrayList should be 1 after removing the specified range`);
}
