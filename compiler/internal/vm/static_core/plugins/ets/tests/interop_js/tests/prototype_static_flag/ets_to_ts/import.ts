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

const stsVm = globalThis.gtest.etsVm;
const Animal = stsVm.getClass('Lexport/Animal;');
const staticRecord = stsVm.getClass('Lexport/ETSGLOBAL;').staticRecord;
const stdObject = stsVm.getClass('Lstd/core/Object;');

class Dog extends Animal {
    breed: string;

    constructor(name: string, age: number, breed: string) {
        super(name, age);
        this.breed = breed;
    }
}

const animal = new Animal('animal', 10);
const dog = new Dog('dog', 3, 'golden');

function foo(arg: Record<'a' | 'b', number>, val1: number, val2: number): Record<'a' | 'b', number> {
    arg.a = val1;
    arg.b = val2;
    return arg;
}

let dynamicRecord: Record<'c' | 'd', number> = { c: 1, d: 2 };

function testStaticProxyFlagOfStaticClass(): void {
    ASSERT_EQ(Object.prototype.hasOwnProperty.call(stdObject, '_isStaticProxy'), true);
    ASSERT_EQ(stdObject._isStaticProxy, true);

    ASSERT_EQ(Object.prototype.hasOwnProperty.call(Animal, '_isStaticProxy'), true);
    ASSERT_EQ(Object.getPrototypeOf(Animal), stdObject);
    ASSERT_EQ(Object.getPrototypeOf(Animal)._isStaticProxy, true);
    ASSERT_EQ(Animal._isStaticProxy, true);
}

function testStaticProxyFlagOfStaticObject(): void {
    ASSERT_EQ(Object.prototype.hasOwnProperty.call(animal.constructor.prototype, '_isStaticProxy'), true);
    ASSERT_EQ(animal.constructor.prototype._isStaticProxy, true);
}

function testStaticProxyFlagOfDynamicClass(): void {
    ASSERT_EQ(Object.prototype.hasOwnProperty.call(Dog, '_isStaticProxy'), false);
}

function testStaticProxyFlagOfDynamicObject(): void {
    ASSERT_EQ(Object.prototype.hasOwnProperty.call(dog.constructor.prototype, '_isStaticProxy'), false);
}

function testStaticProxyFlagOfStaticRecord(): void {
    ASSERT_EQ(Object.prototype.hasOwnProperty.call(Object.getPrototypeOf(staticRecord), '_isStaticProxy'), true);
    ASSERT_EQ(Object.getPrototypeOf(staticRecord)._isStaticProxy, true);
}

function testStaticProxyFlagOfDynamicRecord(): void {
    ASSERT_EQ(Object.prototype.hasOwnProperty.call(Object.getPrototypeOf(dynamicRecord), '_isStaticProxy'), false);
    ASSERT_EQ(Object.getPrototypeOf(dynamicRecord)._isStaticProxy, undefined);
}

testStaticProxyFlagOfStaticClass();
testStaticProxyFlagOfStaticObject();
testStaticProxyFlagOfDynamicClass();
testStaticProxyFlagOfDynamicObject();
testStaticProxyFlagOfStaticRecord();
testStaticProxyFlagOfDynamicRecord();