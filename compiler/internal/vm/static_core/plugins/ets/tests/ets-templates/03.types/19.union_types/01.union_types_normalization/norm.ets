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

{%- for c in cases %}
{%- for k, v in c.items() %}
{%- if k != 'decl' %}
{%- for t in v %}
/*---
desc: >-
    Union types normalization allows minimizing the number of types and literals
    within a union type while keeping the type's safety. Some types or literals
    can also be replaced for more general types.
tags: [{% if k == 'neg'%}compile-only, negative{% endif %}]
---*/
{{c.decl}}

function main() {
    {{t|indent}}
}
{%- endfor %}
{%- endif %}
{%- endfor %}
{%- endfor %}
