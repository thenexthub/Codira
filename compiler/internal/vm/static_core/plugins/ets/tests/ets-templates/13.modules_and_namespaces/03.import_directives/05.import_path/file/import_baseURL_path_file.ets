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
desc: The base URL setting is used to resolve a path that starts with /
files:
    - module_a.ets
    - nesteddir/module_a.ets
    - ../module_a.ets
---*/

import * as all from "/../plugins/ets/tests/ets-templates/13.modules_and_namespaces/03.import_directives/05.import_path/file/module_a.ets"

function main() {
  arktest.assertEQ(all.importedFromFile(), "module_a.ets")
}
