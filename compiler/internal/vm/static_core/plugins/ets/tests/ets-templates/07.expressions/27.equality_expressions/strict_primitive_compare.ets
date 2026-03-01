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

{% for ee in primitives %}

/*---
desc: Strict equality for primitive types
---*/

function strictCompare(a: {{ee.type}}, b: {{ee.type}}): void {
    arktest.assertTrue(a === b)
}

function main(): void {
    let a: {{ee.type}} = {{ee.value}};
    let b: {{ee.type}} = {{ee.value}};

    strictCompare(a, b)
}

{% endfor %}
