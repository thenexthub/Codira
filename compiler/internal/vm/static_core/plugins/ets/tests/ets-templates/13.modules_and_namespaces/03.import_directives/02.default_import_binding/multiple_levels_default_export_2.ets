/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
desc: Import default entity from module A, imported from module B being non-default.
files:
- ../../04.top-level_declarations/01.exported_declarations/default_exported_declarations/default_export_type_2_level_2.ets
---*/

import VAR_i from "../../04.top-level_declarations/01.exported_declarations/default_exported_declarations/default_export_type_2_level_2"

function main() {
    let c: VAR_i = 3;
    arktest.assertEQ(c, 3);
}
