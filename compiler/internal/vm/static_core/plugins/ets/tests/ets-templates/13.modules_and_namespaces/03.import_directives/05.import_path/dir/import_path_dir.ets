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
    If the import path refers to a folder, then the way to handle the case is determined by the actual compiler.
    If the compiler cannot locate a module source definitely, then a compile-time error occurs.
    {{stmt.doc}}
files:
    - ./module_a.ets
    - ./module_a/module_a.ets
    - ./module_b/module_b.ets
---*/

{{stmt.import}}

{% endfor %}
