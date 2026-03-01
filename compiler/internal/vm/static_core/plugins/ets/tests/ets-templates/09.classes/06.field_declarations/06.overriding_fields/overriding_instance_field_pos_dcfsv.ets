/*---
Copyright (c) 2026 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: >-
    When extending a class, an instance field declared in a superclass
    can be overridden by a same-name, same-type field.
    The new declaration adds no new field to the class type but allows
    changing field initialization.
params: different covariant function subtype values
---*/

class A {
  v: int
  constructor(v: int = 0) {
    this.v = v
  }
}

class B extends A {
  constructor(v: int = 0) {
    super(-v)
  }
  f(): int {
    return 13
  }
}

type HandlerBInt = (a: B, b: int) => A

type HandlerAInt = (a: A, b: int) => B

const handlerAInt: HandlerAInt = (a: A, b: int): B => {
  if (a instanceof B){
    return new B(a.f() + b)
  } else {
    return new B(b)
  }
}

type HandlerA = (a: A) => A

const handlerA: HandlerA = (a: A): A => {
  if (a instanceof B){
    return new A(a.f())
  } else {
    return new A
  }
}

type HandlerNothing = () => B

const handlerNothing: HandlerNothing = (): B => {
  return new B(567)
}

class Base {
  field: HandlerBInt = handlerAInt
}

class Derived1 extends Base {
  override field: HandlerBInt = handlerA
}

class Derived2 extends Derived1 {
  override field: HandlerBInt = handlerNothing
}

function get_result(a: Base, b: B, c: int): A {
  return a.field(b, c)
}

function main(): void {
  let a: Derived2 = new Derived2()
  let b: Derived1 = new Derived1()
  let c: Base = new Base()
  arktest.assertTrue(a.field instanceof HandlerNothing)
  arktest.assertEQ(a.field(new B, 7).v, -567)
  arktest.assertEQ((get_result(a, new B, 7)).v, -567)
  arktest.assertTrue(b.field instanceof HandlerA)
  arktest.assertEQ(b.field(new B, 7).v, 13)
  arktest.assertEQ((get_result(b, new B, 7)).v, 13)
  arktest.assertTrue(c.field instanceof HandlerAInt)
  arktest.assertEQ(c.field(new B, 7).v, -20)
  arktest.assertEQ((get_result(c, new B, 7)).v, -20)
}
