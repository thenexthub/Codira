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
desc: |-
    Module imports default entity A from another module, then exports own default entity B.
    Only one default export is allowed in a module.
files:
- ../../04.top-level_declarations/01.exported_declarations/default_exported_declarations/default_export_variable.ets
---*/

import test_var from "../../04.top-level_declarations/01.exported_declarations/default_exported_declarations/default_export_variable"

export default let i: string = test_var;

function main() {
  arktest.assertEQ(i, test_var)
}
