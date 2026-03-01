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

let keepAlive: object[] = [];

class TestClassA1 {}
function testCountInstancesOfClass() {
  const type = Class.from<TestClassA1>()
  let init = RuntimeDebug.countInstancesOfClass(type);
  keepAlive.push(new TestClassA1());
  keepAlive.push(new TestClassA1());
  let count = RuntimeDebug.countInstancesOfClass(type);
  arktest.assertLE(2, count);
  arktest.assertLE(count, init + 2);
}

class TestClassA2 {}
class TestSubClassB2 extends TestClassA2 {}
function testInheritanceBaseClassCount() {
  const type = Class.from<TestClassA2>()
  let init = RuntimeDebug.countInstancesOfClass(type);
  keepAlive.push(new TestClassA2());
  keepAlive.push(new TestClassA2());
  keepAlive.push(new TestSubClassB2());
  let count = RuntimeDebug.countInstancesOfClass(type);
  arktest.assertLE(2, count);
  arktest.assertLE(count, init + 2);
}

class TestClassA3 {}
class TestSubClassB3 extends TestClassA3 {}
function testInheritanceSubClassCount() {
  const type = Class.from<TestSubClassB3>()
  let init = RuntimeDebug.countInstancesOfClass(type);
  keepAlive.push(new TestSubClassB3());
  keepAlive.push(new TestSubClassB3());
  keepAlive.push(new TestClassA3());
  let count = RuntimeDebug.countInstancesOfClass(type);
  arktest.assertLE(2, count);
  arktest.assertLE(count, init + 2);
}

class TestUnusedClassC4 {}
function testZeroInstanceCase() {
  const type = Class.from<TestUnusedClassC4>()
  let init = RuntimeDebug.countInstancesOfClass(type);
  keepAlive.push(new TestUnusedClassC4());
  let count = RuntimeDebug.countInstancesOfClass(type);
  arktest.assertLE(1, count);
  arktest.assertLE(count, init + 1);
}

class IsolatedA {}
class IsolatedB {}
function testMultipleClassIsolation() {
  const typeA = Class.from<IsolatedA>()
  const typeB = Class.from<IsolatedB>()
  let initA = RuntimeDebug.countInstancesOfClass(typeA);
  let initB = RuntimeDebug.countInstancesOfClass(typeB);
  keepAlive.push(new IsolatedA());
  keepAlive.push(new IsolatedA());
  keepAlive.push(new IsolatedB());
  let countA = RuntimeDebug.countInstancesOfClass(typeA);
  arktest.assertLE(2, countA);
  arktest.assertLE(countA, initA + 2);
  let countB = RuntimeDebug.countInstancesOfClass(typeB);
  arktest.assertLE(1, countB);
  arktest.assertLE(countB, initB + 1);
}

class Inner {}
class Outer {
  field: Inner;
  constructor() {
    this.field = new Inner();
  }
}
 
function testNestedObjectCount() {
  const type = Class.from<Inner>()
  let init = RuntimeDebug.countInstancesOfClass(type);
  keepAlive.push(new Outer());
  keepAlive.push(new Outer());
  let count = RuntimeDebug.countInstancesOfClass(type);
  arktest.assertLE(2, count);
  arktest.assertLE(count, init + 2);
}

class Box1<T> {
  value: T;
  constructor(v: T) {
    this.value = v;
  }
}

function testGenericClassCount() {
  const type = Class.from<Box1<string>>()
  let init = RuntimeDebug.countInstancesOfClass(type);
  keepAlive.push(new Box1<string>('a'));
  keepAlive.push(new Box1<string>('b'));
  let count = RuntimeDebug.countInstancesOfClass(type);
  arktest.assertLE(2, count);
  arktest.assertLE(count, init + 2);
}

class NamedAnonymousLike {}
function testAnonymousClassCount() {
  const type = Class.from<NamedAnonymousLike>()
  let init = RuntimeDebug.countInstancesOfClass(type);
  keepAlive.push(new NamedAnonymousLike());
  keepAlive.push(new NamedAnonymousLike());
  let count = RuntimeDebug.countInstancesOfClass(type);
  arktest.assertLE(2, count);
  arktest.assertLE(count, init + 2);
}

function main(): int {
  const suite = new arktest.ArkTestsuite('RuntimeDebug: countInstancesOfClass (Isolated)');
  suite.addTest('Basic', testCountInstancesOfClass);
  suite.addTest('Inheritance: Base only', testInheritanceBaseClassCount);
  suite.addTest('Inheritance: Subclass only', testInheritanceSubClassCount);
  suite.addTest('Zero-instance', testZeroInstanceCase);
  suite.addTest('Multiple classes: IsolatedA vs IsolatedB', testMultipleClassIsolation);
  suite.addTest('Nested object field', testNestedObjectCount);
  suite.addTest('Generic type: Box1<string>', testGenericClassCount);
  suite.addTest('Anonymous class pattern', testAnonymousClassCount);
  keepAlive = [];
  GC.startGC(GC.Cause.FULL);
  return suite.run();
}
