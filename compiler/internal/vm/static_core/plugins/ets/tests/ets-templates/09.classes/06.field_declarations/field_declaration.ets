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
desc: Field declaration.
assert: The variables of a class type are introduces by field declarations.
---*/

class A {
  public i: int;
  public static s: int;
  public readonly b: int = 5;
}

function main(): void {
    let a: A = new A();

    arktest.assertTrue(a.i instanceof int);
    arktest.assertEQ(a.i, 0);
    arktest.assertTrue(A.s instanceof int);
    arktest.assertEQ(A.s, 0);
    arktest.assertTrue(a.b instanceof int);
    arktest.assertEQ(a.b, 5);
}