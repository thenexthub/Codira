/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*---
desc: Checking a protected field from a derived class inside a composite class
---*/

class Person {
  protected name: String = "";
  age: int;
  getName(): string {
    return this.name;
  }
}

class Child extends Person {
  static factory(): Child {
    let child: Child = {name:"Bob", age: 18};

    return child;
  }
}

function main(): void {
  let obj: Child = Child.factory();

  arktest.assertEQ( obj.getName(), "Bob" )
}
