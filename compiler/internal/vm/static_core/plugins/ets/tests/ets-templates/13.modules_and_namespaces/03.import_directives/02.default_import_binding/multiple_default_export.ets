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
desc: Import multiple default entities from different modules.
files:
- ../../04.top-level_declarations/01.exported_declarations/default_exported_declarations/default_export_function.ets
- ../../04.top-level_declarations/01.exported_declarations/default_exported_declarations/default_export_class.ets
---*/

import TestFunc from "../../04.top-level_declarations/01.exported_declarations/default_exported_declarations/default_export_function"
import DefaultExportedClass from "../../04.top-level_declarations/01.exported_declarations/default_exported_declarations/default_export_class"

function main() {
    let c: DefaultExportedClass = new DefaultExportedClass();
    arktest.assertEQ(c.hello("hello"), "hello_world");
    arktest.assertEQ(TestFunc(2), 3);
}
