/*---
Copyright (c) 2025-2026 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: Import union type.
files:
- ../../04.top-level_declarations/01.exported_declarations/type_exported_declarations/export_type_union.ets
---*/

import type {UNI} from "../../04.top-level_declarations/01.exported_declarations/type_exported_declarations/export_type_union"

function main(): void {
    let a: UNI = "str"
    arktest.assertEQ(a, "str")
    a = true
    arktest.assertEQ(a, true)
    a = undefined
    arktest.assertEQ(a, undefined)
}
