/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
desc: Import a default exported class.
files:
- ../../04.top-level_declarations/01.exported_declarations/default_exported_declarations/default_export_class.ets
---*/

import DefaultExportedClass from "../../04.top-level_declarations/01.exported_declarations/default_exported_declarations/default_export_class"

function main(): void {
    let test_class = new DefaultExportedClass();

    arktest.assertEQ(test_class.hello("hello"), "hello_world");
    arktest.assertEQ((test_class.test_member + "_member"), "test_member");
}
