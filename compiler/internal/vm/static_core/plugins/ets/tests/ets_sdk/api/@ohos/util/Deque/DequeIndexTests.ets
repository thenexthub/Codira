/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
import { BusinessError } from "@ohos.base";

const OutOfBoundsErrorCodeId: int = 10200001;

function main(): int {
    const suite = new arktest.ArkTestsuite("Deque API tests");

    suite.addTest("Deque index getter", testIndexGetter);
    suite.addTest("Deque index setter", testIndexSetter);
    suite.addTest("Deque index out of range", testIndexOutOfRange);

    return suite.run();
}

const testNumbers: Number[] = [17, 3, 22, 8, 29, 11, 5, 19, 2, 26, 14, 7, 28, 10, 1, 24, 16, 13,
        21, 6, 27, 18, 4, 25, 12, 23, 9, 15, 20, 30];

function testIndexGetter() {
    let deque = new Deque<double>();
    initializeDeque(deque, testNumbers);
    for (let i: int = 0; i < deque.length; i++) {
        arktest.assertEQ(deque[i], testNumbers[i], `Element at index ${i} should match`);
    }
}

function testIndexSetter() {
    let deque = new Deque<double>();
    initializeDeque(deque, testNumbers);
    for (let i: int = 0; i < deque.length; i++) {
        deque[i] = 3;
    }
    let forEachCount = 0;
    deque.forEach((value: double) => {
        arktest.assertEQ(value, 3, `Element should match`);
        forEachCount++;
    });
    arktest.assertEQ(deque.length, forEachCount, "Deque length should match forEach count");
}

function testIndexOutOfRange() {
    let deque = new Deque<double>();
    initializeDeque(deque, testNumbers);
    let exceptionCheck = (e: Error): boolean | string => {
        if (e instanceof BusinessError) {
            return e.name == "BusinessError" && e.message ==
                `The value of \"index\" is out of range. It must be >= 0 && <= 29. Received value is: -30` &&
                e.code == OutOfBoundsErrorCodeId;
        }
        return false;
    }
    arktest.expectThrow(() => {deque[-testNumbers.length]}, exceptionCheck);
    arktest.expectThrow(() => {deque[-testNumbers.length] = 0}, exceptionCheck);
}

function initializeDeque<T>(deque: Deque<T>, sourceArray: T[]) {
    for (let i: int = 0; i < sourceArray.length; i++) {
        deque.insertEnd(sourceArray[i]);
    }
}
