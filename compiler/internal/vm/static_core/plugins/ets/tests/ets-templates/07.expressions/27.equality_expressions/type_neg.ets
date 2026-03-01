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

{%- for op in operators %}
    {%- for type in types %}
        {%- for operand in operands %}

/*---
desc: Any equality expression is of type boolean
tags: [compile-only, negative]
---*/

const a: {{operand}}
const b: {{operand}}

const e: {{type}} = (a {{op}} b)

// Auxiliary stuff

class C {}
interface I {}

// union
type U = number|string
// tuple
type TU = [number, boolean]
// function
type F = () => void

enum EN { a, b, c }
enum ES { a = "a", b = "b", c = "c" }

        {%- endfor %}
    {%- endfor %}
{%- endfor %}
