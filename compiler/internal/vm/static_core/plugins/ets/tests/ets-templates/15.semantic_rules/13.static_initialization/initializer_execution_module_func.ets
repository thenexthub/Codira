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
    Static initialization must be performed or started before the first execution of one of the following operations:
    - invocation of a function of the module
    - access to a variable of the module
tags: [no-warmup]
files:
- ./modules/init_module.ets
---*/

import {seqstr} from "./modules/init_module"
import {seq} from "./modules/init_module"

function main() {
  seq("A")
  arktest.assertEQ(seqstr, "MOD:A")
}
