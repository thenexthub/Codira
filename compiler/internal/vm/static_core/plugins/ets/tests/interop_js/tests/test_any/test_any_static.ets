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

import { Foo, fooInstance } from "testDecl";
import { NumberMap, numberMapInstance } from "testDecl";
import { functionArg0, functionArg1, functionArg2 } from "testDecl";
import { Qux, quxInstance } from "testDecl";
import { TestNew0, TestNew1, TestNew2 } from "testDecl";
import { Foobar } from "testDecl";
import {arrNum, arrFill, big_val} from 'testDecl';
import { UserAccount } from 'testDecl'

function testIsInstance(): void {
    let res = fooInstance instanceof Foo;
    arktest.assertEQ(res, true);
}

function testLdByName(): void {
    let bar = fooInstance.bar;
    arktest.assertEQ(bar, 0xcafe);

    let baz = fooInstance.baz;
    arktest.assertEQ(baz, 'hello world');
}

function testLdByIdx(): void {
    let babe = numberMapInstance[1];
    arktest.assertEQ(babe, 0xbabe);
}

function testStByIdx(): void {
    numberMapInstance[1] = 0xabcd;

    let babe = numberMapInstance[1];
    arktest.assertEQ(babe, 0xabcd);
}

function testStByName(): void {
    fooInstance.baz = 'updated';
    let baz = fooInstance.baz;
    arktest.assertEQ(baz, 'updated');
}

function testCall0(): void {
    let cafe = functionArg0();
    arktest.assertEQ(cafe, 0xcafe);
}

function testCallShort(): void {
    let caff = functionArg1(0xcafe);
    arktest.assertEQ(caff, 0xcaff);
}

function testCallRange(): void {
    let caff = functionArg2(0xcafe, 1);
    arktest.assertEQ(caff, 0xcaff);
}

function testCallThis0(): void {
    let cafe = quxInstance.methodArg0();
    arktest.assertEQ(cafe, 0xcafe);
}

function testCallThis1(): void {
    let caff = quxInstance.methodArg1(0xcafe);
    arktest.assertEQ(caff, 0xcaff);
}

function testCallThis2(): void {
    let caff = quxInstance.methodArg2(0xcafe, 1);
    arktest.assertEQ(caff, 0xcaff);
}

function testCallNew0(): void {
    let testNew0Instance = new TestNew0();
    let cafe = testNew0Instance.bar;
    arktest.assertEQ(cafe, 0xcafe);
}

function testCallNew1(): void {
    let testNew1Instance = new TestNew1(0xcafe);
    let cafe = testNew1Instance.bar;
    arktest.assertEQ(cafe, 0xcafe);
}

function testCallNew2(): void {
    let testNew2Instance = new TestNew2(0xcafe, 1);
    let caff = testNew2Instance.bar;
    arktest.assertEQ(caff, 0xcaff);
}

function testCallWithUndefined() {
    let foobarInstance = new Foobar(undefined, `placeholder`);
    arktest.assertEQ(foobarInstance.receivedUndefined, true);
}

function testArray(): void {
    let arr1: Array<Number> = arrNum.with(1, 8);
    arktest.assertEQ(arr1.toString(), '1,8,3,4');
}

function testArrayFill(): void {
    let arr1: Array<Number> = arrFill.fill(8, 1, 3);
    arktest.assertEQ(arr1.toString(), '1,8,8,4');
}

function testBigInt(): void {
    let a: Any = big_val;
    arktest.assertEQ(a, 1234567890n);
}

function testSetter(): void {
    const user = new UserAccount('john', 25);
    let res = false;
    try {
        user.age = -11;
        res = false;
    } catch (e) {
        res = e.message === 'Age cannot be negative';
    }
    try {
        user.username = 'jo';
        res = false;
    } catch (e) {
        res = e.message === 'Username must be at least 3 characters long';
    }
    try {
        user.username = 'jo';
        res = false;
    } catch (e) {
        res = e.message === 'Username must be at least 3 characters long';
    }
    arktest.assertTrue(res);
}

function testAll(): void {
    testIsInstance();
    testLdByName();
    testLdByIdx();
    testStByName();
    testStByIdx();
    testCall0();
    testCallShort();
    testCallRange();
    testCallThis0();
    testCallThis1();
    testCallThis2();
    testCallNew0();
    testCallNew1();
    testCallNew2();
    testCallWithUndefined();
    testArray();
    testArrayFill();
    testBigInt();
    testSetter();
}