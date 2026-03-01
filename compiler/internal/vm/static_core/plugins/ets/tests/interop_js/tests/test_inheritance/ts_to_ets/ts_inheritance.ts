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

const etsVm = globalThis.gtest.etsVm;
const Base = etsVm.getClass('Ltest_inheritance/Base;');
const testField = etsVm.getFunction('Ltest_inheritance/ETSGLOBAL;', 'testField');
const testCoverField = etsVm.getFunction('Ltest_inheritance/ETSGLOBAL;', 'testCoverField');
const testGetter = etsVm.getFunction('Ltest_inheritance/ETSGLOBAL;', 'testGetter');
const testSetter = etsVm.getFunction('Ltest_inheritance/ETSGLOBAL;', 'testSetter');
const testMethod = etsVm.getFunction('Ltest_inheritance/ETSGLOBAL;', 'testMethod');
const testSetMethod = etsVm.getFunction('Ltest_inheritance/ETSGLOBAL;', 'testSetMethod');
const testSuperMethod = etsVm.getFunction('Ltest_inheritance/ETSGLOBAL;', 'testSuperMethod');
const testOverloadMethod = etsVm.getFunction('Ltest_inheritance/ETSGLOBAL;', 'testOverloadMethod');
const testOverrideMethod = etsVm.getFunction('Ltest_inheritance/ETSGLOBAL;', 'testOverrideMethod');
const baseAge = etsVm.getClass('Ltest_inheritance/ETSGLOBAL;').baseAge;
const baseFirstName = etsVm.getClass('Ltest_inheritance/ETSGLOBAL;').baseFirstName;
const baseSecondName = etsVm.getClass('Ltest_inheritance/ETSGLOBAL;').baseSecondName;
const fooFirstName = 'BaseFoo';
const fooSecondName = 'Foo';
const fooThirdName = 'Bar';
const fooAge = 10;

class Foo extends Base {
    secondName = fooSecondName;
    thirdName = fooThirdName;
    getFullName(): string {
        return super.getFullName() + '.' + this.thirdName;
    }
}

function checkField(): void {
    let foo = new Foo();
    ASSERT_TRUE(foo.firstName === baseFirstName);
    ASSERT_TRUE(testField(foo));
}

function checkCoverField(): void {
    let foo = new Foo();
    ASSERT_TRUE(foo.secondName === fooSecondName);
    ASSERT_TRUE(testCoverField(foo, fooSecondName));
}

function checkGetter(): void {
    let foo = new Foo();
    ASSERT_TRUE(foo.age === baseAge);
    ASSERT_TRUE(testGetter(foo));
}

function checkSetter(): void {
    let foo = new Foo();
    foo.age = fooAge;
    ASSERT_TRUE(foo.age === fooAge);
    ASSERT_TRUE(testSetter(foo, fooAge));
}

function checkMethod(): void {
    let foo = new Foo();
    ASSERT_TRUE(foo.getFirstName() === baseFirstName);
    ASSERT_TRUE(testMethod(foo));
}

function checkSetMethod(): void {
    let foo = new Foo();
    foo.setFirstName(fooFirstName);
    ASSERT_TRUE(foo.getFirstName() === fooFirstName);
    ASSERT_TRUE(testSetMethod(foo, fooFirstName));
}

function checkSuperMethod(): void {
    let foo = new Foo();
    ASSERT_TRUE(foo.getSecondName() === fooSecondName);
    ASSERT_TRUE(testSuperMethod(foo, fooSecondName));
}

function checkOverloadMethod(): void {
    let foo = new Foo();
    ASSERT_TRUE(testOverloadMethod(foo, 'FooPre'));
}

function checkOverrideMethod(): void {
    let foo = new Foo();
    ASSERT_TRUE(testOverrideMethod(foo, fooThirdName));
}

checkField();
checkCoverField();
checkGetter();
checkSetter();
checkMethod();
checkSetMethod();
checkSuperMethod();
checkOverloadMethod();
checkOverrideMethod();