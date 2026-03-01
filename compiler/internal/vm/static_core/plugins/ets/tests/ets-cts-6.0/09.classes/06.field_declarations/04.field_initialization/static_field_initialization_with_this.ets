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
desc: Static field initialization with keyword 'this'.
assert: >-
    It there is an initializer in a static field declaration, then the following rules apply to the initializer.
    It is a compile-time error if the keyword this or keyword super occurs in the initializer.
tags: [compile-only, negative]
---*/

class A {
  public y: int = 5;

  public static x: int = this.y;
}
