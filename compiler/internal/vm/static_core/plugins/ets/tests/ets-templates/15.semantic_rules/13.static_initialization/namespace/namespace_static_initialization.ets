/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
desc: >-
      For namespaces, the code in a static block is executed only when namespace members are used in the program
      (an example is provided in Namespace Declarations).
      Use static initializer of namespace
tags: [no-warmup]
---*/

namespace ns {
  export class A {
    static f: int
  }
  export class C {
    static s: int = 1
    static {
      A.f = 3
    }
  }
  static {
    A.f = 2
  }
  export let i: string = ""
}

function main() {
  arktest.assertEQ(ns.A.f, 0)
  arktest.assertEQ(ns.C.s, 1)
  arktest.assertEQ(ns.A.f, 3)
  ns.i = "str"
  arktest.assertEQ(ns.A.f, 2)
}
