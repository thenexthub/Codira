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
{% for cm in top_level_class_modifiers %}
/*---
desc: >-
    If possible instantiations need to be constrained, then an individual constraint can be set for each type parameter after
    the keyword extends. A constraint can have the form of any type.
    {{c.doc}}
tags: [compile-only]
params: >
    {{c.type}}
    {{cm}}
---*/

{{c.decl}}

{{cm}} class Point<T extends {{c.type}}> {}

function main(): void {
{% if "abstract" not in cm %}
    new Point<{{c.use}}>()
{% endif %}
}
{% endfor %}
{% endfor %}
