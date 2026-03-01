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
      If binding alias is present, then the identifier specified in binding alias is used.
      The bounded entity is no longer accessible (see Accessible) under the original name.
tags: [compile-only, negative]
files:
- ./modules/example.ets
---*/

import {f1 as f2} from "./modules/example.ets"

function main() {
  f1(1)
}
