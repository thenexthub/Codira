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
desc: ambient import entities from multiple modules using bind all method
files:
- ./modules/mod1.ets
- ./modules/mod2.ets
---*/

import * as A from "./modules/mod1.d"
import * as B from "./modules/mod2.d"

function main() {
  arktest.assertEQ(A.foo(), 1)
  arktest.assertEQ(A.f, 2)
  arktest.assertEQ(new A.C().foo(), 3)
  arktest.assertEQ(B.foo(), 4)
  arktest.assertEQ(B.f, 5)
  arktest.assertEQ(new B.C().foo(), 6)
}
