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
desc: Multiple member inheritance.
assert: There might be several paths by which the same member type declaration is inherited from an interface. In such a situation, the member type is considered to be inherited only once.
tags: [compile-only, negative]
---*/

interface A {
  class C {
    public static bar(): int {
      return 10;
    }
  }
}

class B {
  class C {
    public static bar(): int {
      return 5;
    }
  }
}

class D extends B implements A {
}
