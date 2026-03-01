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

{% for c in cases %}
/*---
desc: Record index expression - spread from a record cast from Object.
---*/

let y: Record<string, string> = {
    "name": "Alice",
    "age": "30"
}

function spreadFromObject(y: Record<string, string>): Record<string, string> {
    let x: Record<string, string>  = { ...y };
    return x;
}

function getValue(key: string, defaultValue: string): string {
    let x = spreadFromObject(y);
    return x[key] ?? defaultValue;
}

function main(): void {
    let x = spreadFromObject(y);
    arktest.assertEQ({{c.call}}, {{c.value}});
}
{% endfor %}
