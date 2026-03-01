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
desc: >-
  If each initialization expression is of a numeric type (see Numeric Types),
  then the type of the array literal is number[]
---*/

{{c.decl}}

type my_type = {%- if c.type is defined %} {{c.type}} {%- else %} Numeric {%- endif %}

function main() {
    let x = {{c.val}}
    arktest.assertTrue( x instanceof Array, "failed instanceof Array" )
    for(let idx = 0; idx < x.length; idx++) {
        arktest.assertTrue(x[idx] instanceof my_type)
    }
}
{% endfor %}
