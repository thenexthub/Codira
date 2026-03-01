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
desc: no target type is expected in the context, then the expression is called a standalone expression.
      The type of some expressions cannot be inferred from the expression itself
tags: [compile-only, negative]
---*/

class A {
    name: string = ""
    age: number = 0
    {% if c.class %}{{c.class}}{% endif %}
}

class B { 
    id: number = 0
    name: string = ""
}

let a = {{c.global}}; // no target type is expected

{% endfor %}
