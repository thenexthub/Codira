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

const etsVm = globalThis.gtest.etsVm;
class TestModule {
    descriptorPrefix: string;
    constructor(name) {
      this.descriptorPrefix = 'L' + name.replaceAll('.', '/') + '/';
    }

    getClass(name): Object { return etsVm.getClass(this.descriptorPrefix + name + ';'); }
    getFunction(name): Object { return etsVm.getFunction(this.descriptorPrefix + 'ETSGLOBAL;', name); }

    static descriptorPrefix;
  };

function getTestModule(name): Object { return new TestModule(name); }

const testModule = getTestModule('interface_method_return_value');

export function test(): Object {
    const directClass = testModule.getClass('InstanceClass');
    const dir = new directClass();
    return true;
}

export function getLiteralTypeReturn(): Object {
    const getterFn = testModule.getFunction('getBoolValue');
    const classInstance = getterFn();
    const expectedFalse = classInstance.getBoolean();
    return typeof expectedFalse === 'boolean' && !expectedFalse;
}

export function getNativeArrayReturn(): Object {
    const getterFn = testModule.getFunction('getArrayValue');
    const classInstance = getterFn();
    const result = classInstance.getArray();
    const [destructuredFirst] = result;
    const canDestructureCorrectly = destructuredFirst === result[0];
    return Array.isArray(result) && canDestructureCorrectly;
}

export function getRecordTypeReturn(): Object {
    const getterFn = testModule.getFunction('getRefValue');
    const classInstance = getterFn();
    const returnedRecord = classInstance.getRecord();
    const shouldBeValidFirst = returnedRecord[0][0] === 'child_0';
    const isMissingOptionUndefined = returnedRecord[1];
    return shouldBeValidFirst && isMissingOptionUndefined;
}

export function getRefTypeReturn(): Object {
    const getterFn = testModule.getFunction('getRefValue');
    const classInstance = getterFn();
    const canGetInterfaceMethod = Boolean(classInstance.getLiteral) && typeof classInstance.getLiteral === 'function';
    const canGetNotDescribedMethod = Boolean(classInstance.methodNotDeclaredInInterface);
    let failsOnInexistentMethod: boolean = false;

    try {
        classInstance.methodYouDontHave();
    } catch {
        failsOnInexistentMethod = true;
    }
    return canGetInterfaceMethod && canGetNotDescribedMethod && failsOnInexistentMethod;
}

export function getUnion(): Object {
    const getterFn = testModule.getFunction('getUnion');
    // @ts-ignore
    const instance = getterFn();
    const foundResults: (number | boolean | string)[] = [];
    while (!foundResults.includes(1234) && !foundResults.includes(false) && !foundResults.includes('stringValue')) {
        const runResult = instance.getUnion();
        if (!foundResults.includes(runResult)) {
            foundResults.push(runResult);
        }
    }
    return true;
}
