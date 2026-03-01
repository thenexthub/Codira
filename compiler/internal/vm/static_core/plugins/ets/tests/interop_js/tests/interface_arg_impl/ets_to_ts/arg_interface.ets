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

interface Callable {
    call(): string;
}

interface Ageable extends Callable {
    get age(): int;
}

interface Foo extends Ageable {
    foo(): boolean;
}

interface AnotherInterface {
    run(): boolean;
}

function testFoo(val: Foo): boolean {
    arktest.assertTrue(val instanceof Foo);
    arktest.assertTrue(val instanceof Ageable);
    arktest.assertTrue(val instanceof Callable);
    return val.call() === '1' && val.age === 1 && val.foo();
}

function testAgeable(val: Ageable): boolean {
    arktest.assertTrue(val instanceof Ageable);
    arktest.assertTrue(val instanceof Callable);
    return val.call() === '1' && val.age === 1;
}

function testCallable(val: Callable): boolean {
    return val.call() === '1';
}

function testMultiInterface(val: AnotherInterface): boolean {
    arktest.assertTrue(val instanceof Foo);
    arktest.assertTrue(val instanceof Ageable);
    arktest.assertTrue(val instanceof Callable);
    arktest.assertTrue(val instanceof AnotherInterface);
    let obj: Foo = val as Foo;
    return obj.call() === '1' && obj.age === 1 && obj.foo() && !val.run();
}