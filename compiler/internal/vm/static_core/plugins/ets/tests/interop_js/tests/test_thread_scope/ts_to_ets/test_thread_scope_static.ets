/*
* Copyright (c) 2025 Huawei Device Co., Ltd.
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

import { Foo, fooInstance, fooFunc, testObject, testStr, testNumber } from 'testDecl';

function testGetObjectPropertis(): boolean {
    let f1 = fooInstance;
    for (let i = 0; i < 200000; i++) {
        let value = f1.objectProperty;
        if (value === undefined) {
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
    let f1 = fooInstance;
    for (let i = 0; i < 200000; i++) {
        let value = f1.stringProperty;
        if (value !== testStr) {
            return false;
        }
    }
    return true;
}

function testSetStringPropertis(): boolean {
    let f1 = fooInstance;
    for (let i = 0; i < 200000; i++) {
        f1.stringProperty = testStr;
    }
    return true;
}

function testGetNumberPropertis(): boolean {
    let f1 = fooInstance;
    for (let i = 0; i < 200000; i++) {
        let value = f1.numberProperty;
        if (value !== testNumber) {
            return false;
        }
    }
    return true;
}

function testSetNumberPropertis(): boolean {
    let f1 = fooInstance;
    for (let i = 0; i < 200000; i++) {
        f1.numberProperty = testNumber;
    }
    return true;
}


function testCallFooFunc(): boolean {
    let f1 = fooFunc;
    let f2 = fooInstance;
    for (let i = 0; i < 200000; i++) {
        f1(f2);
    }
    return true;
}