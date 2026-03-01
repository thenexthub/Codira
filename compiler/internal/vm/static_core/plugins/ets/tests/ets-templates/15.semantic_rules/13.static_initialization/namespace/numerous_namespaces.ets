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
      Static initializers from multiple namespaces work independently
---*/

namespace ns1 {
  export let i: string
  static {
    i = "ns1 initialized"
  }
}

namespace ns2 {
  export let i: string
  static {
    i = "ns2 initialized"
  }
}

namespace ns3 {
  export let i: int
  static {
    i = 2
  }
}

function main() {
  arktest.assertEQ(ns1.i, "ns1 initialized")
  arktest.assertEQ(ns2.i, "ns2 initialized")
  arktest.assertEQ(ns3.i, 2)
}
