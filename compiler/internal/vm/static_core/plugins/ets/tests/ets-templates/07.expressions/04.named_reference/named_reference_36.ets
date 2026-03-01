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
desc: Cross-module variable reference by qualified name.
files:
- ./named_reference_export8.ets
---*/

import {PI, CONFIG} from './named_reference_export8.ets';

function main(): void {
  arktest.assertEQ(PI, 3.1415);
  arktest.assertEQ(CONFIG.env, "production");
}