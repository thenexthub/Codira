/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
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

package PreallocTest;

function fillHeap(arr: Object[], numberOfIter: number): void
{
    let globalIdx = 0;
    const objectSize = GC.getObjectSize(new Object());
    const count = Math.pow(2, 20) / (objectSize / 8);

    arr[globalIdx++] = new Object[count.toInt()];
    arr[globalIdx++] = new Object[(count / 2).toInt()];
    arr[globalIdx++] = new Object[(count / 32).toInt()];
    arr[globalIdx++] = new Object[(count / 64).toInt()];
    arr[globalIdx++] = new Object[(count / 128).toInt()];
    arr[globalIdx++] = new Object[(count / 256).toInt()];

    let newIterNmb = numberOfIter - globalIdx;
    for (let idx = 0; idx < newIterNmb; ++idx) {
        arr[globalIdx++] = new Object;
    }

    throw new Error("ets_vm_init_prealloc_test.ets: unsuccessful attempt of fill the heap.");
}

function compareErrorObjects(
    error: Error,
    expectedErrorName: string,
    expectedErrorMsg: string,
    expectedErrorStack: string
): boolean
{
    if (error.name !== expectedErrorName) {
        console.log(`Expected '${expectedErrorName}' error name, actual '${error.name}'`);
        return false;
    }

    if (error.message !== expectedErrorMsg) {
        console.log(`Expected '${expectedErrorMsg}' error message, actual '${error.message}'`);
        return false;
    }

    if (error.stack !== expectedErrorStack) {
        console.log(`Expected '${expectedErrorStack}' error stack, but actual '${error.stack}'`);
        return false;
    }

    let expectedErrorToString = error.name + ((error.name !== "" && error.message)? ": " + error.message: error.message);
    if (error.toString() !== expectedErrorToString) {
        console.log(`Expected '${expectedErrorToString}' error stack, but actual '${error.toString()}'`);
        return false;
    }

    return true;
}

function testThrowingPreallocOOMObjectSuite(): Error
{
    let numberOfIter = 900000;
    let arr = new Object[numberOfIter];

    let error: Error;
    try {
        fillHeap(arr, numberOfIter);
    } catch(e) {
        if (e instanceof OutOfMemoryError){
            return e;
        }
    }

    return new Error();
}

function testThrowingPreallocOOMObject(
    expectedErrorName: string,
    expectedErrorMsg: string,
    expectedErrorStack: string
): boolean
{
    let error = testThrowingPreallocOOMObjectSuite();

    GC.startGC(GC.Cause.FULL);

    return compareErrorObjects(error, expectedErrorName, expectedErrorMsg, expectedErrorStack);
}

function testThrowingPreallocOOMObjectTwiceSuite(): Error
{
    let numberOfIter = 900000;
    let arr = new Object[numberOfIter];
    let arr2: Object;

    let error: Error = new Error("testThrowingPreallocOOMObjectTwiceSuite failed");
    try {
        fillHeap(arr, numberOfIter);
    } catch(e) {
        if (!(e instanceof OutOfMemoryError)) {
            throw e;
        }
    }

    try {
        // Try to allocate more memory to get one more OOM
        arr2 = new Object[numberOfIter];
        fillHeap(arr2, numberOfIter);
    } catch(e) {
        if (e instanceof OutOfMemoryError){
            error = e;
        }
    }

    return error;
}

function testThrowingPreallocOOMObjectTwice(
    expectedErrorName: string,
    expectedErrorMsg: string,
    expectedErrorStack: string
): boolean
{
    let error = testThrowingPreallocOOMObjectTwiceSuite();

    GC.startGC(GC.Cause.FULL);

    return compareErrorObjects(error, expectedErrorName, expectedErrorMsg, expectedErrorStack);
}
