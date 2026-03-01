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
params: different covariant subtype values
---*/

class Animal {
  name: string = ""
}

class Dog extends Animal {
  override name: string = "dog"
}

class Cat extends Animal {
  override name: string = "cat"
}

class Base {
  field: Animal = new Dog
}

class A extends Base {
  override field: Animal = new Cat
}

function main(): void {
  let a: A = new A()
  let b: Base = new Base()
  arktest.assertTrue(a.field instanceof Cat)
  arktest.assertEQ(a.field.name, "cat")
  arktest.assertTrue(b.field instanceof Dog)
  arktest.assertEQ(b.field.name, "dog")
}
