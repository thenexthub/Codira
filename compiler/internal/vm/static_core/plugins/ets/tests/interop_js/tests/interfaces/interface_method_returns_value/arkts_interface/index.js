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
'use strict';

globalThis.gtest.etsVm = requireNapiPreview('lib/ets_interop_js_napi.so', false);

Object.defineProperty(exports, '__esModule', { value: true });


const etsVm = globalThis.gtest.etsVm;

const TestModule = /** @class */ (function () {
    function TestModule(name) {
        this.descriptorPrefix = 'L' + name.replaceAll('.', '/') + '/';
    }
    TestModule.prototype.getClass = function (name) { return etsVm.getClass(this.descriptorPrefix + name + ';'); };
    TestModule.prototype.getFunction = function (name) {
         return etsVm.getFunction(this.descriptorPrefix + 'ETSGLOBAL;', name);
    };
    return TestModule;
}());

function getTestModule(name) { return new TestModule(name); }
const testModule = getTestModule('interface_method_return_value');

function test() {
    let ahcGetter = testModule.getFunction('getThroughInterface');
    let lol = ahcGetter();
    let directClass = testModule.getClass('InstanceClass');
    let dir = new directClass();
    return true;
}

export function getLiteralTypeReturn() {
    let getterFn = testModule.getFunction('getBoolValue');
    let classInstance = getterFn();
    let expectedFalse = classInstance.getBoolean();
    return typeof expectedFalse === 'boolean' && !expectedFalse;
}

export function getNativeArrayReturn() {
    let getterFn = testModule.getFunction('getArrayValue');
    let classInstance = getterFn();
    let result = classInstance.getArray();
    let destructuredFirst = result[0];
    let canDestructureCorrectly = destructuredFirst === result[0];
    return Array.isArray(result) && canDestructureCorrectly;
}

export function getRecordTypeReturn() {
    let getterFn = testModule.getFunction('getRefValue');
    let classInstance = getterFn();
    let returnedRecord = classInstance.getRecord();
    let shouldBeValidFirst = returnedRecord[0][0] === 'child_0';
    let isMissingOptionUndefined = returnedRecord[1];
    return shouldBeValidFirst && isMissingOptionUndefined;
}

export function getRefTypeReturn() {
    let getterFn = testModule.getFunction('getRefValue');
    let classInstance = getterFn();
    let canGetInterfaceMethod = Boolean(classInstance.getLiteral) && typeof classInstance.getLiteral === 'function';
    let canGetNotDescribedMethod = Boolean(classInstance.methodNotDeclaredInInterface);
    let failsOnInexistentMethod = false;
    try {
        classInstance.methodYouDontHave();
    }
    catch (_a) {
        failsOnInexistentMethod = true;
    }
    return canGetInterfaceMethod && canGetNotDescribedMethod && failsOnInexistentMethod;
}

export function getUnion() {
    let getterFn = testModule.getFunction('getUnion');
    // @ts-ignore
    let instance = getterFn();
    let foundResults = [];
    while (!foundResults.includes(1234) && !foundResults.includes(false) && !foundResults.includes('stringValue')) {
        let runResult = instance.getUnion();
        console.log(runResult);
        if (!foundResults.includes(runResult)) {
            foundResults.push(runResult);
        }
    }
    return true;
}
