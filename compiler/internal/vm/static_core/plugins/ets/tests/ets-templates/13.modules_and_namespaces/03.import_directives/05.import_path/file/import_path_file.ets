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

{% for stmt in import_stmts %}

/*---
desc: >-
    The compiler imports the module that the import path refers to. The source code of the module is the
    file with the extension provided within the import path, or, if none is so provided, appended by the compiler.
files: [./module_a.ets, ./nesteddir/module_a.ets, ../module_a.ets]
---*/

{{stmt.import}}

function main() {
  arktest.assertEQ(all.importedFromFile(), {{stmt.res}})
}

{% endfor %}
