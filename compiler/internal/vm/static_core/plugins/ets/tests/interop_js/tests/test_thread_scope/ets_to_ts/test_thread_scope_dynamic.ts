/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

let etsVm = globalThis.gtest.etsVm;
let fooInstance = etsVm.getClass('Ltest_thread_scope_static/ETSGLOBAL;').fooInstance;
let testObject = etsVm.getClass('Ltest_thread_scope_static/ETSGLOBAL;').testObject;
let testStr = etsVm.getClass('Ltest_thread_scope_static/ETSGLOBAL;').testStr;
let testNumber = etsVm.getClass('Ltest_thread_scope_static/ETSGLOBAL;').testNumber;
let fooFunc = etsVm.getFunction('Ltest_thread_scope_static/ETSGLOBAL;', 'fooFunc');

function testGetObjectPropertis(): boolean {
    for (let i = 0; i < 200000; i++) {
        let value = fooInstance.objectProperty;
        if (value !== testObject) {
            return false;
        }
    }
    return true;
}

function testSetObjectPropertis(): boolean {
    for (let i = 0; i < 200000; i++) {
        fooInstance.objectProperty = testObject;
    }
    return true;
}

function testGetStringPropertis(): boolean {
    for (let i = 0; i < 200000; i++) {
        let value = fooInstance.stringProperty;
        if (value !== testStr) {
            return false;
        }
    }
    return true;
}

function testSetStringPropertis(): boolean {
    for (let i = 0; i < 200000; i++) {
        fooInstance.stringProperty = testStr;
    }
    return true;
}

function testGetNumberPropertis(): boolean {
    for (let i = 0; i < 200000; i++) {
        let value = fooInstance.numberProperty;
        if (value !== testNumber) {
            return false;
        }
    }
    return true;
}

function testSetNumberPropertis(): boolean {
    for (let i = 0; i < 200000; i++) {
        fooInstance.numberProperty = testNumber;
    }
    return true;
}


function testCallFooFunc(): boolean {
    for (let i = 0; i < 200000; i++) {
        let value = fooFunc(fooInstance);
        if (value !== fooInstance) {
            return false;
        }
    }
    return true;
}

ASSERT_TRUE(testGetObjectPropertis());
ASSERT_TRUE(testSetObjectPropertis());
ASSERT_TRUE(testGetStringPropertis());
ASSERT_TRUE(testSetStringPropertis());
ASSERT_TRUE(testGetNumberPropertis());
ASSERT_TRUE(testSetNumberPropertis());
ASSERT_TRUE(testCallFooFunc());