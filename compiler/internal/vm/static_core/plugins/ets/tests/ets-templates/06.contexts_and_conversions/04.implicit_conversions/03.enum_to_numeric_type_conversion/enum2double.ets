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

{%- for c in cases %}
/*---
desc: >-
    A value of the numeric enumeration type is converted to one of the following:
    - Numeric type equal to or larger than the enumeration base type
    - Union type considering the value type of the enumeration base type
---*/
enum Color: double { Red = 0.0, Green = 100.0, Blue = 101.0 }

{{c.decl}}

function main() {
    {{c.use|indent}}
}
{%- endfor %}
