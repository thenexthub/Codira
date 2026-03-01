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
tags: [no-warmup]
---*/

let seqstr = ""
function seq(i: string) {
  seqstr += ":" + i
}

namespace ns1 {
  export let d: number = 1
  static {
    seq("A")
  }
  export namespace ns2 {
    export let i: number = 2
    static {
      seq("B")
    }
  }
}

function main() {
  arktest.assertEQ(seqstr, "")
  ns1.d
  arktest.assertEQ(seqstr, ":A")
  ns1.ns2.i
  arktest.assertEQ(seqstr, ":A:B")
}
