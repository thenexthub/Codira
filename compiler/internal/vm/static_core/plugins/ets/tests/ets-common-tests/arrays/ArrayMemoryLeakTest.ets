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

class TestClass {}

function __noinline__addData(arr: TestClass[]): WeakRef<TestClass> {
    let data = new TestClass();
    for (let i = 0; i < 5; i++) {
        arr.push(new TestClass());
    }
    arr.push(data);
    return new WeakRef<TestClass>(data);
}

function popElement(arr: TestClass[]) {
    while (arr.length > 0) {
        arr.pop();
    }
}

function testArrayPopGC() {
    let arr: TestClass[] = [];
    let wr = __noinline__addData(arr);
    arktest.expectNoThrow(() => { GC.waitForFinishGC(GC.startGC(GC.Cause.FULL)); });
    arktest.assertNE(wr.deref(), undefined);

    popElement(arr);
    arktest.expectNoThrow(() => { GC.waitForFinishGC(GC.startGC(GC.Cause.FULL)); });
    arktest.assertEQ(wr.deref(), undefined);
}

function resetLength(arr: TestClass[]) {
    arr.length = 0;
}

function testArrayLengthGC() {
    let arr: TestClass[] = [];
    let wr = __noinline__addData(arr);
    arktest.expectNoThrow(() => { GC.waitForFinishGC(GC.startGC(GC.Cause.FULL)); });
    arktest.assertNE(wr.deref(), undefined);

    resetLength(arr);
    arktest.expectNoThrow(() => { GC.waitForFinishGC(GC.startGC(GC.Cause.FULL)); });
    arktest.assertEQ(wr.deref(), undefined);
}

function shiftElement(arr: TestClass[]) {
    while (arr.length > 0) {
        arr.shift();
    }
}

function testArrayShiftGC() {
    let arr: TestClass[] = [];
    let wr = __noinline__addData(arr);
    arktest.expectNoThrow(() => { GC.waitForFinishGC(GC.startGC(GC.Cause.FULL)); });
    arktest.assertNE(wr.deref(), undefined);

    shiftElement(arr);
    arktest.expectNoThrow(() => { GC.waitForFinishGC(GC.startGC(GC.Cause.FULL)); });
    arktest.assertEQ(wr.deref(), undefined);
}

function spliceAndPopOne(arr: TestClass[]) {
    arr.splice(1, 1);
    arr.pop();
}

function testArraySpliceGC() {
    let arr: TestClass[] = [];
    let wr = __noinline__addData(arr);
    arktest.expectNoThrow(() => { GC.waitForFinishGC(GC.startGC(GC.Cause.FULL)); });
    arktest.assertNE(wr.deref(), undefined);

    spliceAndPopOne(arr);
    arktest.expectNoThrow(() => { GC.waitForFinishGC(GC.startGC(GC.Cause.FULL)); });
    arktest.assertEQ(wr.deref(), undefined);
}

function main(): int {
    const suite = new arktest.ArkTestsuite('Array memory leak tests');
    suite.addTest('for pop GC', testArrayPopGC);
    suite.addTest('for <set>length GC', testArrayLengthGC);
    suite.addTest('for shift GC', testArrayShiftGC);
    suite.addTest('for splice GC', testArraySpliceGC);
    return suite.run();
}
