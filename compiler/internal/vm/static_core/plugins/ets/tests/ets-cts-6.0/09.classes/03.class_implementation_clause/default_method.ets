/*---
Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
desc: Override default method from superinterface
assert: >-
    Each default method of a superinterface of the class may optionally be overridden by a method in the class;
    if not, the default method is typically inherited and its behavior is as specified by its default body.
---*/

class A implements C {
  public override foo(): int {
    return 10;
  }
}

interface C {
  foo(): int {
    return 5;
  };
}

function main(): void {
  let a: A = new A()

  arktest.assertEQ(a.foo(), 10);
}
