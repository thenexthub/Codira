/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE- 2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

const etsVm = globalThis.gtest.etsVm;

let testFoo = etsVm.getFunction('Larg_interface/ETSGLOBAL;', 'testFoo');
let testAgeable = etsVm.getFunction('Larg_interface/ETSGLOBAL;', 'testAgeable');
let testCallable = etsVm.getFunction('Larg_interface/ETSGLOBAL;', 'testCallable');
let testMultiInterface = etsVm.getFunction('Larg_interface/ETSGLOBAL;', 'testMultiInterface');

class MyImpl {
    constructor() {
        'implements static:Larg_interface/Foo;,Larg_interface/AnotherInterface;';
    }
    foo(): boolean {
        return true;
    }
    get age(): number {
        return 1;
    }
    call(): string {
        return '1';
    }
    run(): boolean {
        return false;
    }
}

class NotImpl {}

function checkSingleObject(): void {
    ASSERT_TRUE(testFoo(new MyImpl()));
    ASSERT_TRUE(testAgeable(new MyImpl()));
    ASSERT_TRUE(testCallable(new MyImpl()));
    ASSERT_TRUE(testMultiInterface(new MyImpl()));
}

function checkCommonObj(): void {
    let obj = new MyImpl();
    ASSERT_TRUE(testFoo(obj));
    ASSERT_TRUE(testAgeable(obj));
    ASSERT_TRUE(testCallable(obj));
    ASSERT_TRUE(testMultiInterface(obj));
}

function checkNotImplError() {
    try {
        testFoo(new NotImpl());
    } catch (e: Error) {
        ASSERT_TRUE(e.toString() === 'TypeError: object is not a type of Interface: Larg_interface/Foo;');
    }
}

checkSingleObject();
checkCommonObj();
checkNotImplError();
