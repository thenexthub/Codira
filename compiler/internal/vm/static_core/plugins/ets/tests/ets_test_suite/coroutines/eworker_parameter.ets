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

class TestA {
    public a:string = '';
}

function tesObject(a:TestA) : TestA {
    a.a = 'bbb';
    return a;
}

function testReturnArray(a:TestA, b:TestA, c:string) : Array<TestA> {
    a.a = 'bbb';
    b.a = c;
    let arr = new Array<TestA>();
    arr.push(a);
    arr.push(b);
    return arr;
}

function testArrayObjects(arr:Array<TestA>) : Array<TestA> {
    arr[0].a = 'value1';
    arr[1].a = 'value2';
    return arr;
}

function testPrimitive(a:int, b:string, c:boolean, d:double) {
    arktest.assertEQ(a, 1);
    arktest.assertEQ(b, 'bbb');
    arktest.assertEQ(c, true);
    arktest.assertEQ(d, 1.0);
}

function singleObject():void {
    let job = eaw.run<TestA>(tesObject, new TestA());
    arktest.assertEQ(job.Await().a, 'bbb');
}

function returnArray():void {
    let a1 = new TestA();
    let a2 = new TestA();
    let job = eaw.run<Array<TestA>>(testReturnArray, a1, a2, 'ccc');
    let arr = job.Await();
    arktest.assertEQ(arr[0].a, 'bbb');
    arktest.assertEQ(arr[1].a, 'ccc');
    arktest.assertEQ(a1.a, 'bbb');
    arktest.assertEQ(a2.a, 'ccc');
}

function arrayParameter():void {
    let arr:Array<TestA> = new Array<TestA>();
    let a1 = new TestA();
    let a2 = new TestA();
    arr.push(a1);
    arr.push(a2);
    let job = eaw.run<Array<TestA>>(testArrayObjects, arr);
    let arr2 = job.Await();
    arktest.assertEQ(arr2[0].a, 'value1');
    arktest.assertEQ(arr2[1].a, 'value2');
    arktest.assertEQ(arr2, arr);
    arktest.assertEQ(a1.a, 'value1');
    arktest.assertEQ(a2.a, 'value2');
}

function primitiveParameter() : void {
    eaw.run<void>(testPrimitive, 1, 'bbb', true, 1.0).Await();
}

function undefinedTest() {
    let callback = (arg?: int): boolean => {
        if (arg == undefined) {
            return true;
        }
        return false;
    }
    let job = eaw.run<boolean>(callback, undefined);
    let res = job.Await();
    arktest.assertEQ(res, true);
}

let eaw:EAWorker = new EAWorker();

function main(): int {
    eaw.start();
    const testSuite = new arktest.ArkTestsuite('EAWorkerParameterTest');
    testSuite.addTest('singleObject', singleObject);
    testSuite.addTest('returnArray', returnArray);
    testSuite.addTest('arrayParameter', arrayParameter);
    testSuite.addTest('primitiveParameter', primitiveParameter);
    testSuite.addTest("undefinedTest", undefinedTest);
    let res = testSuite.run();
    eaw.join();
    return res;
}
