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

function sumInt(value1: int, value2: int): int {
    return value1 + value2;
}

async function taskpoolFuncTestSumInt() {
    let result = await taskpool.execute(sumInt, 10, 20);
    arktest.assertEQ(result, 30);
}

async function taskpoolFuncTestAnd() {
    let and = (value1: boolean, value2: boolean): boolean => {
        if (value1 & value2) {
            return true;
        }
        return false;
    }
    let result = await taskpool.execute(and, true, true);
    arktest.assertEQ(result, true);
}

async function taskpoolFuncTestStrCat() {
    let strCat = (value1: string, value2: string): string => {
        return value1 + value2;
    }
    let result = await taskpool.execute(strCat, 'abc', 'def');
    arktest.assertEQ(result, 'abcdef');
}

async function taskpoolFuncTestStrCat2() {
    let strCat = (value1: string, value2: string): string => {
        return value1 + value2;
    }
    let result = await taskpool.execute(strCat, 'abc', 'def');
    result = await taskpool.execute(strCat, 'abc', 'def');
    arktest.assertEQ(result, 'abcdef');
}

async function taskpoolFuncTestStrCat3() {
    let strCat = (value1: string, value2: string): string => {
        return value1 + value2;
    }
    let sum = (value1: string, value2: string): string => {
        return value1 + strCat(value2, 'hello');
    }
    let result = await taskpool.execute(sum, 'abc', 'def');
    arktest.assertEQ(result, 'abcdefhello');
}

async function taskpoolFuncTestArraySum() {
    let sum = (value1: int[], value2: int[]): int[] => {
        let result: int[] = [0, 0];
        result[0] = value1[0] + value2[0];
        result[1] = value1[1] + value2[1];
        return result;
    }
    let result = (await taskpool.execute(sum, [1, 2], [3, 4])) as int[];
    arktest.assertEQ(result[0], 4);
    arktest.assertEQ(result[1], 6);
}

class A {
    constructor(a: int, b: int) { this.a = a; this.b = b; }
    constructor() { this(0, 0); }
    public a: int;
    public b: int;
}

async function taskpoolFuncTestClassFieldsSum() {
    let sum = (value1: A, value2: A): A => {
        let result = new A();
        result.a = value1.a + value2.a;
        result.b = value1.b + value2.b;
        return result;
    }
    let result = (await taskpool.execute(sum, new A(1, 2), new A(3, 4))) as A;
    arktest.assertEQ(result.a, 4);
    arktest.assertEQ(result.b, 6);
}

async function taskpoolFuncTestSumInt2() {
    let result1 = await taskpool.execute(sumInt, 10, 20);
    let result2 = await taskpool.execute(sumInt, 30, 40);
    arktest.assertEQ(result1, 30);
    arktest.assertEQ(result2, 70);
}

async function taskpoolFuncTestSumInt2SameArguments() {
    let result1 = await taskpool.execute(sumInt, 10, 20);
    let result2 = await taskpool.execute(sumInt, 10, 20);
    arktest.assertEQ(result1, 30);
    arktest.assertEQ(result2, 30);
}

async function taskpoolFuncTestSumAndMulti() {
    let multiInt = (value1: int, value2: int): int => {
        return value1 * value2;
    }
    let result1 = await taskpool.execute(sumInt, 10, 20);
    let result2 = await taskpool.execute(multiInt, 10, 20);
    let result3 = await taskpool.execute(sumInt, 10, 30);
    let result4 = await taskpool.execute(multiInt, 20, 20);
    arktest.assertEQ(result1, 30);
    arktest.assertEQ(result2, 200);
    arktest.assertEQ(result3, 40);
    arktest.assertEQ(result4, 400);
}

function main(): int {
    let funcTaskSuite = new arktest.ArkTestsuite('taskpool.FuncTask');
    funcTaskSuite.addAsyncTest('TaskpoolFuncTestSumInt', taskpoolFuncTestSumInt);
    funcTaskSuite.addAsyncTest('TaskpoolFuncTestAnd', taskpoolFuncTestAnd);
    funcTaskSuite.addAsyncTest('TaskpoolFuncTestStrCat', taskpoolFuncTestStrCat);
    funcTaskSuite.addAsyncTest('TaskpoolFuncTestStrCat2', taskpoolFuncTestStrCat2);
    funcTaskSuite.addAsyncTest('TaskpoolFuncTestStrCat3', taskpoolFuncTestStrCat3);
    funcTaskSuite.addAsyncTest('TaskpoolFuncTestArraySum', taskpoolFuncTestArraySum);
    funcTaskSuite.addAsyncTest('TaskpoolFuncTestClassFieldsSum', taskpoolFuncTestClassFieldsSum);
    funcTaskSuite.addAsyncTest('TaskpoolFuncTestSumInt2', taskpoolFuncTestSumInt2);
    funcTaskSuite.addAsyncTest('TaskpoolFuncTestSumInt2SameArguments', taskpoolFuncTestSumInt2SameArguments);
    funcTaskSuite.addAsyncTest('TaskpoolFuncTestSumAndMulti', taskpoolFuncTestSumAndMulti);
    let res = funcTaskSuite.run();
    CoroutineExtras.stopTaskpool();
    return res;
}
