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

let module = ESValue.load('test_class')
let getId = module.getProperty('getId');
let testClass = module.getProperty('TestClass');

function getProperty(): boolean {
    const testObj = testClass.instantiate('OneTestClass', 123);
    return testObj.getProperty('name').toString() === 'OneTestClass' &&
        testObj.getProperty('id').toNumber() === 123;
}

function useStringFn(): boolean {
    const testObj = testClass.instantiate('OneTestClass', 123);
    return testObj.invokeMethod('greet').toString() === 'This is OneTestClass';
}

function useNumberFn(): boolean {
    const testObj = testClass.instantiate('OneTestClass', 123);
    return testObj.invokeMethod('myId').toNumber() === 123;
}

function useExternalFn(): boolean {
    const testObj = testClass.instantiate('OneTestClass', 123);
    const objId = getId.invoke(testObj);
    return objId.toNumber() === 123;
}
