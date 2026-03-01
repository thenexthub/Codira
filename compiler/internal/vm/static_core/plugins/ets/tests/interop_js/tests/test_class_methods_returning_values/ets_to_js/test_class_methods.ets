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

package class_methods_test


const TEST_STRING = 'This is a test string';
const TEST_INT = 100;
const TEST_BOOLEAN = true;

function functionReturnStringSubsetByRef(): string{
    return TEST_STRING;
}

function functionReturnMapSubsetByRef() {
    let TEST_CHANGEABLE_MAP: Map<number, string>  = new Map<number, string>();
    TEST_CHANGEABLE_MAP.set(1, 'Test')
    TEST_CHANGEABLE_MAP.set(2, 'Test')
    return TEST_CHANGEABLE_MAP;
}

function functionReturnStringSubsetByValue() {
    return TEST_STRING;
}

function functionReturnIntSubsetByValue() {
    return TEST_INT;
}

function functionReturnBoolSubsetByValue() {
    return TEST_BOOLEAN;
}

function functionReturnUndefinedSubsetByValue() {
    return;
}

function functionReturnNullSubsetByValue() {
    return null;
}

type UnionNumberOrString = int | string;

function functionReturnUnion(is_str: boolean): UnionNumberOrString {
    if (is_str) {
        return '1000';
    }
    return 1000;
}

class TestClass {
    testNumber: number;
    testString: string;
  
    constructor(testNumber: number, testString: string) {
      this.testNumber = testNumber;
      this.testString = testString;
    }
    
    testFunction(): number {
       return this.testNumber;
    }

    public static isTestClass(instance: TestClass): boolean {
        return instance instanceof TestClass;
      }
}

function functionReturnClass(): TestClass {
    const testClass: TestClass = new TestClass(1, 'Test');
    return testClass;
}

// Note: The following code for using of interface is commented until #17878 is not resolved
// Note: TypeError: target type for class composite TestInterface is not instantiable
/*
interface TestInterface {
    testNumber: number;
    testString: string;
}

let testInterfaceVal: TestInterface = {
    testNumber: 100,
    testString: 'Test'
}

function functionReturnInterface(): TestInterface {
    return testInterfaceVal; 
}*/

// Note: Using this code for interface is a workaround until #17878 is not resolved
interface InterfaceWithMethodEts {
    testNumber: int;
    methodInInterface(): int;
}

class ImplInterfaceWithMethodEts implements InterfaceWithMethodEts {
    testNumber: int;

    constructor(testNumber: int) {
        this.testNumber = testNumber;
    }

    public override methodInInterface(): int {
        return this.testNumber;
    }
}

function newInterfaceWithMethodEts(): InterfaceWithMethodEts {
    return new ImplInterfaceWithMethodEts(100);
}
