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
desc: >-
    Import a default exported function using its name in a selective import.
    TypeError Use the default import syntax to import a default exported element
files:
- ../../04.top-level_declarations/01.exported_declarations/default_exported_declarations/default_export_function.ets
tags:
- compile-only
- negative
---*/

import {TestFunc as TestFunc} from "../../04.top-level_declarations/01.exported_declarations/default_exported_declarations/default_export_function"

function main(): void {
    TestFunc(42);
}
