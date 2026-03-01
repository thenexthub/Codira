{%- for c in cases %}
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
    ArkTS provides extended semantics for conditional-and and conditional-or expressions for better alignment.
---*/

{{c.decl}}

function main(): void {
    let v: {{c.type}} = {{c.val}}
    let res: int = 0;
    if (v) {  // truthy
    } else {
        res = 1
    }
    arktest.assertEQ(res, 0)
}
{%- endfor %}
