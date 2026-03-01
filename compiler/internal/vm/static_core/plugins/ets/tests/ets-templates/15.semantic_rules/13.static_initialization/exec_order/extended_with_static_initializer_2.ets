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
      Static initialization is performed before the first execution of one of the following operations
      - Static initialization of a direct subclass of an entity that is a class.
tags: [no-warmup]
---*/

let seqstr = ""
function seq(i: string) {
  seqstr += ":" + i
}

class A {
  static {
    seq("init A")
  }
}

class B extends A { }

class C extends B {
  static {
    seq("init C")
  }
}

function main() {
  arktest.assertEQ(seqstr, "")
  new C()
  arktest.assertEQ(seqstr, ":init A:init C")
}
