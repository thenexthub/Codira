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
let module = ESValue.load('js_import')
let primitive = module.getProperty('primitive');
let TestObject = module.getProperty('TestObject');
let testfn = module.getProperty('testfn');
let ManyArgsClass = module.getProperty('ManyArgsClass');
let fnTestUser = module.getProperty('fnTestUser');
let SubClass = module.getProperty('SubClass');
let TwoArgClass = module.getProperty('TwoArgClass');
let OneArgClass = module.getProperty('OneArgClass');
let NoArgClass = module.getProperty('NoArgClass');

function noArgsClass(): boolean {
    const testObj = NoArgClass.instantiate();
    return testObj.getProperty('message').toString() === 'Class without args';
}

function noArgsClassBrackes(): boolean {
    const testObj = NoArgClass.instantiate();
    return testObj.getProperty('message').toString() === 'Class without args';
}

function oneArgsClass(): boolean {
    const testObj = OneArgClass.instantiate('TestName');
    return testObj.getProperty('name').toString() === 'TestName';
}

function twoArgsClass(): boolean {
    const testObj = TwoArgClass.instantiate('TestName', 'TestCity');
    return testObj.getProperty('city').toString() === 'TestCity' && testObj.getProperty('name').toString() === 'TestName';
}
function manyArgsClass(): boolean {
    const testObj = ManyArgsClass.instantiate('TestName', 30, 'US', 'Boston');
    return testObj.invokeMethod('showInfo').toString() === 'Name: TestName, age:30, country: US, city: Boston';
}

function newWithSubClass(): boolean {
    const testObj = SubClass.instantiate('TestName', 'Test surname');
    return testObj.getProperty('name').toString() === 'TestName' && testObj.getProperty('surname').toString() === 'Test surname';
}

function newWithException(): boolean {
    const testException = new Error('Test error');
    return testException.message === 'Test error';
}
function newWithFunction(): boolean {
    const testObj = fnTestUser.instantiate('Test name', 'Surname test');
    return testObj.getProperty('name').toString() === 'Test name' && testObj.getProperty('surname').toString() === 'Surname test';
}

function newWithArrey(): boolean {  
    let testArr: Array<String> = new Array<String>();
    testArr.push('Test');
    return testArr[0] === 'Test';
}

function newWithPrototype(): boolean {
    const testObj = fnTestUser.instantiate('Fill', 'Smith');
    return testObj.invokeMethod('showInfo').toString() === 'Name: Fill, surname: Smith';
}

// Negative cases
function newWithJustFunction(): boolean {
    try {
        const instance = testfn.instantiate();
        return false
    } catch (e) {
        return true
    }
}

function newWithObj(): boolean {
    try {
        const instance = TestObject.instantiate();
        return false
    } catch (e) {
        return true
    }
}

function newWithPrimitive(): boolean {
    try {
        const newPrimitive = primitive.instantiate();
        return false
    } catch (e) {
        return true
    }   
}