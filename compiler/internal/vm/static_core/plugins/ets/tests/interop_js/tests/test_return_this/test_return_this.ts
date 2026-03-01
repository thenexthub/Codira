/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

const TEST_ARRAY: number[] = [1, 2, 3, 4, 5];
const TEST_STRING: string = 'Test';
const TEST_NUMBER: number = 100;
const TEST_NULL: null = null;
const TEST_MAP: Map<string, string> = new Map<string, string>([['TEST', 'test']]);
const TEST_TUPLE: [string, number] = ['Test', 1];
const TEST_OBJ: Object = true;
type AnyType = {};

export class TestClass {
    testString: string;
    testNumber: number;

    constructor() {
        this.testString = TEST_STRING;
        this.testNumber = TEST_NUMBER;
      }

    getThis(): this {
        return this;
    }
}

export function getThis(this: AnyType): AnyType {
    return this;
}

export function getThisAsFunction(this: Function): Function { 
    return this;
}

export async function getThisAsAsyncFunction(this: Function): Promise<Function> { 
    return this;
}

export function testFuncReturnStr(): string {
    return TEST_STRING;
}

export class ReturnThisTestClass {
    public getThis = getThis.bind(this);
    public getThisAsFunction = getThisAsFunction.bind(this);
    public getThisAsAsyncFunction = getThisAsAsyncFunction.bind(this);
}

export type TReturnThisTestClass = ReturnThisTestClass;
export const returnThisTestClassInstance = new ReturnThisTestClass();

const testClass = (): void => {
    let testClass = new TestClass();
    let testVal = testClass.getThis();
};

const test = (): void => {
    console.log(
        getThis.call(TEST_NUMBER),
        getThis.call(TEST_STRING),
        getThis.call(TEST_NULL),
        getThis.call(TEST_MAP),
        getThis.call(TEST_OBJ),
        getThis.call(TEST_ARRAY),
        getThis.call(TEST_TUPLE),
        getThis.call(new TestClass()),
        getThisAsFunction.call(testFuncReturnStr())
    );
};

const testAsync = async (): Promise<void> => {
    console.log(await getThisAsAsyncFunction.call(testFuncReturnStr()));
};

testClass();
test();
testAsync();
