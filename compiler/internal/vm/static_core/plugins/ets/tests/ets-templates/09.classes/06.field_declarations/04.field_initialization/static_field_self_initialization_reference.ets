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
desc: Non-static field 'f' have reference to itself.
assert: >-
    For a reference to a static field f declared in class or interface C, it is a compile-time error if.
    The reference appears in the initializer of f's own declaration or at a point prior to f's declaration.
tags: [compile-only, negative]
---*/

class C {
  public static f: int = C.f + 5;
}
