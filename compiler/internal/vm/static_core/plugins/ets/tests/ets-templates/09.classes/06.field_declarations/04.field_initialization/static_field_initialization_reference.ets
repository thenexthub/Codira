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
desc: Static field 'f' have reference in static variable.
assert: >-
    For a reference to a static field f declared in class or interface C, it is a compile-time error if.
    The reference appears either in a static variable's initializer of C or in a static initializer of C.
tags: [compile-only, negative]
---*/

class C {
  public static a: int = C.f + 5;
  public static f: int = C.a;
}
