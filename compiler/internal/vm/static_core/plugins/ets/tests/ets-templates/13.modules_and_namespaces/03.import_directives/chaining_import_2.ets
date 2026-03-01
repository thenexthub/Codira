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
desc: Chaining import (default -> selective -> all) all import step
files:
- ./chaining_import_1.ets
name: chaining_import_2
---*/

import * as Test from "./chaining_import_1"
import {default as DefaultExportedClass} from "../04.top-level_declarations/01.exported_declarations/default_exported_declarations/default_export_class"

export {default as SomeExp} from "./chaining_import_0"

function main() {
arktest.assertEQ(new Test.SomeExportedClass1().hello('hello'), 'hello_world')
    arktest.assertEQ(Test.testvar, 1)
}
