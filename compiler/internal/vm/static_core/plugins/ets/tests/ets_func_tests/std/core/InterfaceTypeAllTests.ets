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

interface IEmpty { }
interface IOne { foo(): void; }
interface IMulti extends IOne {
    bar(x: int): string;
    baz(): boolean;
}

class ImplementsEmpty implements IEmpty { }
class ImplementsOne implements IOne {
    foo(): void {}
}
class ImplementsMulti implements IMulti {
    foo(): void {}
    bar(x: int): string { return "" + x; }
    baz(): boolean { return true; }
}

class NoImpl { }

function main(): int {
    const suite = new arktest.ArkTestsuite('InterfaceType API tests for SDK');

    suite.addTest('hasName/getName for IEmpty', testEmptyHasNameGetName);
    suite.addTest('hasName/getName for IOne', testOneHasNameGetName);
    suite.addTest('hasName/getName for IMulti', testMultiHasNameGetName);
    suite.addTest('getInterfacesNum zero', testZeroInterfaces);
    suite.addTest('getInterfacesNum one', testOneInterface);
    suite.addTest('getInterfacesNum multi', testMultiInterfaceCount);
    suite.addTest('getInterface out-of-range', testInterfaceOutOfRange);
    suite.addTest('getMethodsNum for IEmpty', testMethodsNumEmpty);
    suite.addTest('getMethodsNum for IOne', testMethodsNumOne);
    suite.addTest('getMethodsNum for IMulti', testMethodsNumMulti);
    suite.addTest('getMethod valid', testGetMethodValidIndex);
    suite.addTest('getMethod out-of-range', testGetMethodOutOfRange);

    return suite.run();
}

function toInterfaceType<T>(instance: T): InterfaceType {
    const cls = Type.of(instance) as ClassType;
    const count = cls.getInterfacesNum();
    arktest.assertTrue(count > 0, `Expected at least one interface, but got ${count}`);
    return cls.getInterface(0) as InterfaceType;
}

function testEmptyHasNameGetName() {
    const it = toInterfaceType(new ImplementsEmpty());
    arktest.assertTrue(it.hasName());
    arktest.assertEQ('InterfaceTypeAllTests.IEmpty', it.getName());
}
function testOneHasNameGetName() {
    const it = toInterfaceType(new ImplementsOne());
    arktest.assertTrue(it.hasName());
    arktest.assertEQ('InterfaceTypeAllTests.IOne', it.getName());
}
function testMultiHasNameGetName() {
    const it = toInterfaceType(new ImplementsMulti());
    arktest.assertTrue(it.hasName());
    arktest.assertEQ('InterfaceTypeAllTests.IMulti', it.getName());
}

function testZeroInterfaces() {
    const t = Type.of(new NoImpl()) as ClassType;
    arktest.assertEQ(0 as long, t.getInterfacesNum());
}
function testOneInterface() {
    const t = Type.of(new ImplementsOne()) as ClassType;
    arktest.assertEQ(1 as long, t.getInterfacesNum());
    arktest.assertEQ('InterfaceTypeAllTests.IOne', t.getInterface(0).getName());
}
function testMultiInterfaceCount() {
    const t = Type.of(new ImplementsMulti()) as ClassType;
    arktest.assertEQ(1 as long, t.getInterfacesNum());
    arktest.assertEQ('InterfaceTypeAllTests.IMulti', t.getInterface(0).getName());
}
function testInterfaceOutOfRange() {
    const t = Type.of(new ImplementsOne()) as ClassType;
    let msg = '';
    try { t.getInterface(1); } catch (e) { msg = `${e}`; }
    arktest.assertEQ('TypeError: no interface at 1', msg);
}

function testMethodsNumEmpty() {
    const it = toInterfaceType(new ImplementsEmpty());
    arktest.assertEQ(0 as long, it.getMethodsNum());
}
function testMethodsNumOne() {
    const it = toInterfaceType(new ImplementsOne());
    arktest.assertEQ(1 as long, it.getMethodsNum());
}
function testMethodsNumMulti() {
    const it = toInterfaceType(new ImplementsMulti());
    arktest.assertEQ(3 as long, it.getMethodsNum());
}

function testGetMethodValidIndex() {
    const it = toInterfaceType(new ImplementsMulti());
    const n = it.getMethodsNum();
    const m0 = it.getMethod(0);
    arktest.assertEQ(true, m0.getName().length > 0);
    const mLast = it.getMethod(n - 1 as long);
    arktest.assertEQ(true, mLast.getName().length > 0);
}
function testGetMethodOutOfRange() {
    const it = toInterfaceType(new ImplementsMulti());
    const n = it.getMethodsNum();
    const m0 = it.getMethod(0);
    arktest.assertTrue(m0.getName().length > 0);
    const mLast = it.getMethod(n - 1 as long);
    arktest.assertTrue(mLast.getName().length > 0);
}
