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

/*---
desc: Subclasses override superclass methods with union type arguments and return type.
---*/

class Base1 {}
class Base2 {}
class Child extends Base2 {
  childField = 111;
  constructor(a: Int) {
    this.childField = a;
  }
}
class TestBase1 {
    TestField: Base1 | Base2 | undefined = new Base2()
    foo(a: Base1 | Base2) {
        return 1;
    }
    bar(a: Int): Base1 | Base2 | undefined {
      return this.TestField;
    }
}
class Test1 extends TestBase1 {
    TestField2: Base1 | Base2 | undefined = new Base1()
    override foo(a: Base2 | Base1) {
        return 2;
    }
    override bar(a: Int): Base1 | Base2 | undefined {
      return this.TestField2;
    }
}

function test1() {
  let test = new Test1();
  arktest.assertEQ(test.foo(new Base1()), 2);
  arktest.assertEQ(test.foo(new Base2()), 2);
  arktest.assertEQ(test.foo(new Child(123)), 2);
  arktest.assertTrue(test.bar(1) instanceof Base1);
}

class TestBase2 {
    TestField: Base1 | Base2 | undefined = undefined
    foo(a: Base1 | Child) {
        return 3;
    }
    bar(a: Int): Base1 | Base2 | undefined {
      return this.TestField;
    }
}

class Test2 extends TestBase2 {
    override foo(a: Base2 | Base1) {
        return 4;
    }
    override bar(a: Int): Child | Base1 | undefined {
        return new Child(a);
    }
}

function test2() {
  let test = new Test2();
  arktest.assertEQ(test.foo(new Base1()), 4);
  arktest.assertEQ(test.foo(new Base2()), 4);
  arktest.assertEQ(test.foo(new Child(123)), 4);
  arktest.assertTrue(test.bar(1) instanceof Child);
}

class Test2_1 extends TestBase2 {
    override foo(a: Base2 | Base1 | Test1) {
        return 5;
    }
}

function test2_1() {
  let test = new Test2_1();
  arktest.assertEQ(test.foo(new Base1()), 5);
  arktest.assertEQ(test.foo(new Base2()), 5);
  arktest.assertEQ(test.foo(new Child(123)), 5);
  arktest.assertEQ(test.foo(new Test1()), 5);
}

interface I {}
class ChildI extends Base2 implements I {}

class TestBase3 {
    foo(a: ChildI) {
        return 6;
    }
    bar(a: Int): Base2 | I {
      if (a > 0) {
        return new C();
      } else {
        return new Base2();
      }
    }
}

class C implements I {}
class Test3 extends TestBase3 {
    override foo(a:  Base2 | I) {
      return 7;
    }
    override bar(a: Int): ChildI {
      return new ChildI();
    }
}

function test3() {
  let test = new Test3();
  arktest.assertEQ(test.foo(new Base2()), 7);
  arktest.assertEQ(test.foo(new C()), 7);
  arktest.assertEQ(test.foo(new ChildI()), 7);
  arktest.assertTrue(test.bar(1) instanceof ChildI);
}
class Child2 extends Base2 {}
class TestBase4 {
  foo(a: Child | Child2) {
    return 8;
  }
  bar(a: Int): Base2 | undefined {
    if (a > 0) {
      return new Base2();
    }
    return undefined;
  }
}

class Test4 extends TestBase4 {
  override foo(a: Base2) {
    return 9;
  }
  override bar(a: Int): Child | Child2| undefined {
    if (a > 0) {
      return new Child2();
    }
    return new Child(123);
  }
}

function test4() {
  let test = new Test4();
  arktest.assertEQ(test.foo(new Base2()), 9);
  arktest.assertEQ(test.foo(new Child(123)), 9);
  arktest.assertEQ(test.foo(new Child2()), 9);
  arktest.assertTrue(test.bar(1) instanceof Child2);
  arktest.assertTrue(test.bar(-1) instanceof Child);
}

class TestBase5 {
  foo(a: Base2 | Child) {
    return 10;
  }
  bar(a: Int): Base2 | undefined{
    if (a > 0) {
      return new Base2();
    }
    return undefined;
  }
}

class Test5 extends TestBase5 {
  override foo(a: Base2) {
    return 11;
  }
  override bar(a: Int): Base2 | Child | undefined{
    if (a > 0) {
      return new Child(123);
    }
    return new Base2();
  }
}

function test5() {
  let test = new Test5();
  arktest.assertEQ(test.foo(new Base2()), 11);
  arktest.assertEQ(test.foo(new Child(123)), 11);
  arktest.assertTrue(test.bar(1) instanceof Child);
  arktest.assertTrue(test.bar(-1) instanceof Base2);
}

function main(): void {
  test1();
  test2();
  test3();
  test4();
  test5();
}
