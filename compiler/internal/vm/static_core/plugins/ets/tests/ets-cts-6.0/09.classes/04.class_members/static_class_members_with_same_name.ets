/*---
Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
    All names in both static and non-static class declaration scopes (see Scopes) must be unique,
    i.e., fields and methods cannot have the same name.
---*/

class A {
  foo: int = 2;
  static foo(): int {
    return 1;
  }
}

function main(): void {
  let i: A = new A();
  arktest.assertEQ( i.foo, 2 );
  arktest.assertEQ( A.foo(), 1 );
}
