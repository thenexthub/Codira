/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
    Any typeof expression is of type string. The typeof expression values of
    the types below are predefined, and the expressions require no evaluation.
    Chapter 3.0 states table of Aliases for predefined types.
---*/
{{c.decl}}

{%- for v in c['vals'] %}
class Z{{loop.index}} {
    fld: string = ""
    constructor(p: {{v.type}}) {
        this.fld = typeof p
    }
    meth(p: {{v.type}}): string {
        return this.fld == typeof p ? this.fld : "error"
    }
}
{%- endfor %}

function main() {
{%- for v in c['vals'] %}
    arktest.assertEQ( new Z{{loop.index}}({{v.expr}}).meth({{v.expr}}), "{{c.res}}" )
{%- endfor %}
}
{%- endfor %}
