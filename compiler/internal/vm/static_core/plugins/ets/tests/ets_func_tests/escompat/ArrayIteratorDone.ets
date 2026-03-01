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

function main(): int {
    let arrayTestsuite = new arktest.ArkTestsuite("arrayTestsuite");
    arrayTestsuite.addTest("valuesDoneStrTest", valuesDoneStrTest);
    arrayTestsuite.addTest("entriesDoneStrTest", entriesDoneStrTest);

    arrayTestsuite.addTest("valuesDoneObjTest", valuesDoneObjTest);
    arrayTestsuite.addTest("entriesDoneObjTest", entriesDoneObjTest);
    return arrayTestsuite.run();
}

function valuesDoneStrTest() {
    let array = new Array<string>();
    let iterator = array.values();
    array.push("a");
    let result = iterator.next();
    arktest.assertEQ(result.done, false, 'First result `done` flag for values() of Array<string>');
    arktest.assertEQ(result.value, "a", 'First result `value` for values() of Array<string>');
    result = iterator.next();
    arktest.assertEQ(result.done, true, 'Exhausted result `done` flag for values() of Array<string>');
    arktest.assertEQ(result.value, undefined, 'Exhausted result `value` for values() of Array<string>');
    array.push("b");
    result = iterator.next();
    arktest.assertEQ(result.done, true, 'Exhausted result `done` flag after push for values() of Array<string>');
    arktest.assertEQ(result.value, undefined, 'Exhausted result `value` after push for values() of Array<string>');
}

function entriesDoneStrTest() {
    let array = new Array<string>();
    let iterator = array.entries();
    array.push("a");
    let result  = iterator.next();
    arktest.assertEQ(result.done, false, 'First result `done` flag for entries() of Array<string>');
    arktest.assertEQ(result.value![0], 0 , 'First result `value` for entries() of Array<string>');
    arktest.assertEQ(result.value![1], "a", 'First result `value` for entries() of Array<string>');
    result = iterator.next();
    arktest.assertEQ(result.done, true, 'Exhausted result `done` flag for entries() of Array<string>');
    arktest.assertEQ(result.value, undefined, 'Exhausted result `value` for entries() of Array<string>');
    array.push("b");
    result = iterator.next();
    arktest.assertEQ(result.done, true, 'Exhausted result `done` flag after push for entries() of Array<string>');
    arktest.assertEQ(result.value, undefined, 'Exhausted result `value` after push for entries() of Array<string>');
}

class MyObject {
    private a: Int;
    private b: Double;
    constructor(a: Int, b: Double) {
        this.a = a;
        this.b = b;
    }
    public getA(): Int {
        return this.a;
    }
    public getB(): Double{
        return this.b;
    }
}

function valuesDoneObjTest() {
    let obj1 = new MyObject(1, 1.1);
    let obj2 = new MyObject(2, 2.2);
    let array = new Array<MyObject>(obj1, obj2);
    let iterator = array.values();
    let result = iterator.next();
    arktest.assertEQ(result.done, false, 'First result `done` flag for values() of Array<Object>');
    arktest.assertEQ(result.value!.getA(), 1, 'First result `value.a` for values() of Array<Object>');
    arktest.assertEQ(result.value!.getB(), 1.1, 'First result `value.b` for values() of Array<Object>');
    result = iterator.next();
    arktest.assertEQ(result.done, false, 'Second result `done` flag for values() of Array<Object>');
    arktest.assertEQ(result.value!.getA(), 2, 'Second result `value.a` for values() of Array<Object>');
    arktest.assertEQ(result.value!.getB(), 2.2, 'Second result `value.b` for values() of Array<Object>');
    result = iterator.next();
    arktest.assertEQ(result.done, true, 'Exhausted result `done` flag for values() of Array<Object>');
    arktest.assertEQ(result.value, undefined, 'Exhausted result `value` for values() of Array<Object>');
    array.push(new MyObject(3, 3.3));
    result = iterator.next();
    arktest.assertEQ(result.done, true, 'Exhausted result `done` flag after push for values() of Array<Object>');
    arktest.assertEQ(result.value, undefined, 'Exhausted result `value` after push for values() of Array<Object>');
}

function entriesDoneObjTest() {
    let obj1 = new MyObject(1, 1.1);
    let obj2 = new MyObject(2, 2.2);
    let array = new Array<MyObject>(obj1, obj2);
    let iterator = array.entries();
    let result = iterator.next();
    arktest.assertEQ(result.done, false, 'First result `done` flag for entries() of Array<Object>');
    arktest.assertEQ(result.value![0], 0 , 'First result `value[0]` for entries() of Array<Object>');
    arktest.assertEQ(result.value![1].getA(), 1, 'First result `value[1].a` for entries() of Array<Object>');
    arktest.assertEQ(result.value![1].getB(), 1.1, 'First result `value[1].b` for entries() of Array<Object>');
    result = iterator.next();
    arktest.assertEQ(result.done, false, 'Second result `done` flag for entries() of Array<Object>');
    arktest.assertEQ(result.value![0], 1 , 'Second result `value[0]` for entries() of Array<Object>');
    arktest.assertEQ(result.value![1].getA(), 2, 'Second result `value[1].a` for entries() of Array<Object>');
    arktest.assertEQ(result.value![1].getB(), 2.2, 'Second result `value[1].b` for entries() of Array<Object>');
    result = iterator.next();
    arktest.assertEQ(result.done, true, 'Exhausted result `done` flag for entries() of Array<Object>');
    arktest.assertEQ(result.value, undefined, 'Exhausted result `value` for entries() of Array<Object>');
    array.push(new MyObject(3, 3.3));
    result = iterator.next();
    arktest.assertEQ(result.done, true, 'Exhausted result `done` flag after push for entries() of Array<Object>');
    arktest.assertEQ(result.value, undefined, 'Exhausted result `value` after push for entries() of Array<Object>');
}
