/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

function main(): int {
    const suite = new arktest.ArkTestsuite("Array every tests for SDK");
    suite.addTest("Array.every testBasic ", testBasic);
    suite.addTest("Array.every testEmptyArray ", testEmptyArray);
    suite.addTest("Array.every testModifyDuringIteration ", testModifyDuringIteration);
    suite.addTest("Array.every testObjectArray ", testObjectArray);
    suite.addTest("Array.every testEarlyTermination ", testEarlyTermination);
    suite.addTest("Array.every testCallbackError ", testCallbackError);
    suite.addTest("Array.every testImpureCallback ", testImpureCallback);
    return suite.run();
}


function testBasic() {
    const arr: number[] = [1, 2, 3, 4, 5];
    const result1 = arr.every(x => x < 10);
    arktest.assertEQ(result1, true)
    const result2 = arr.every(x => x % 2 === 0);
    arktest.assertEQ(result2, false)
}


function testEmptyArray() {
    const arr: number[] = [];
    const result = arr.every(x => x > 10);
    arktest.assertEQ(result, true)
}


function testModifyDuringIteration() {
    const arr1: number[] = [1, 2, 3];
    let iterations1 = 0;
    const result1 = arr1.every(x => {
        iterations1++;
        arr1.push(4);
        return true;
    });
    arktest.assertEQ(result1, true)
    arktest.assertEQ(iterations1, 3)
    arktest.assertEQ(arr1[0], 1)
    arktest.assertEQ(arr1[1], 2)
    arktest.assertEQ(arr1[2], 3)
    arktest.assertEQ(arr1[3], 4)
    arktest.assertEQ(arr1[4], 4)
    arktest.assertEQ(arr1[5], 4)

    const arr2: number[] = [1,2,3]
    let iterations2 = 0;
    const result2 = arr2.every(x => {
        iterations2++;
        arr2.shift()
        arr2.shift()
        return true;
    });
    arktest.assertEQ(result2, true)
    arktest.assertEQ(iterations2, 1)
    arktest.assertEQ(arr2[0], 3)

    const arr3: number[] = [1,2,3]
    let iterations3 = 0;
    const result3 = arr3.every(x => {
        iterations3++;
        arr3.shift()
        arr3.shift()
        arr3.shift()
        arr3.shift()
        arr3.push(4)
        arr3.push(4)
        arr3.push(4)
        return true;
    });
    arktest.assertEQ(result3, true)
    arktest.assertEQ(iterations3, 3)
    arktest.assertEQ(arr3[0], 4)
    arktest.assertEQ(arr3[1], 4)
    arktest.assertEQ(arr3[2], 4)
}

interface Person {
    name: string;
    age: number;
}

function testObjectArray() {
    const people: Person[] = [
        { name: 'Alice', age: 25 },
        { name: 'Bob', age: 30 },
        { name: 'Charlie', age: 22 }
    ];
    
    const allAdults = people.every(p => p.age >= 18);
    arktest.assertEQ(allAdults, true)
    
    const allNamesStartWithA = people.every(p => p.name.startsWith('A'));
    arktest.assertEQ(allNamesStartWithA, false)
}


function testEarlyTermination() {
    const arr: number[] = [1, 2, 3, 4, 5];
    let visited: number[] = [];
    
    const result = arr.every(x => {
        visited.push(x);
        return x < 3;
    });
    arktest.assertEQ(result, false)
    arktest.assertEQ(visited[0], 1)
    arktest.assertEQ(visited[1], 2)
    arktest.assertEQ(visited[2], 3)
}


function testCallbackError() {
    const arr: Array<number | string> = [1, 2, 3, 'four', 5];
    try {
        const result = arr.every(x => {
            if (typeof x !== 'number') {
                throw new Error('Not number');
            }
            return true;
        });
    } catch (e) {
        arktest.assertEQ(e.toString(), 'Error: Not number')
    }
}


function testImpureCallback() {
    let externalState = 0;
    const arr: number[] = [1, 2, 3, 4, 5];
    
    const result = arr.every(x => {
        externalState += x;
        return externalState < 10;
    });
    arktest.assertEQ(result, false)
    arktest.assertEQ(externalState, 10)
}

