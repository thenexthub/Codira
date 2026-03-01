/*---
* Copyright (c) 2025 Huawei Device Co., Ltd.
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
---*/

{% for c in constraints %}
{% for mm in method_modifiers %}

/*---
desc: >-
    If possible instantiations need to be constrained, then an individual constraint can be set for each type parameter.
    A constraint of any type parameter can follow the keyword extends. The constraint is denoted as any type.
params: >
  {{c.type}}
  {{mm}}
---*/

{{c.decl}}

class Point {
  {{mm}} foo<{{c.type}}>(i: X, j: Z) { return i};
}

function main(): void {
{% if "static" in mm %}
  arktest.assertEQ( Point.foo<{{c.use}}>({{c.p}}), {{c.expected}} )
{% else %}
  arktest.assertEQ( new Point().foo<{{c.use}}>({{c.p}}), {{c.expected}} )
{% endif %}
}

{% endfor %}
{% endfor %}
