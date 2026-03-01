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
desc: Chaining import (default -> selective -> all) Default import
  export-import same class
files:
- ../04.top-level_declarations/01.exported_declarations/default_exported_declarations/default_export_class.ets
name: chaining_import_0
---*/

import {default as DefaultExportedClass0} from "../04.top-level_declarations/01.exported_declarations/default_exported_declarations/default_export_class"

export DefaultExportedClass0

class TestClass {
    hello(param: String): String {
        return param + "_world";
    }

    test_member = "test";
}

class DefClass {
}

export TestClass
export default DefClass

function main() {
    arktest.assertEQ(new TestClass().test_member, new DefaultExportedClass0().test_member)
}