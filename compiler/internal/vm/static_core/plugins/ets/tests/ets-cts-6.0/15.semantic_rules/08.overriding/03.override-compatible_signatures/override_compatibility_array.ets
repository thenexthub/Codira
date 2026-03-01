/*---
Copyright (c) 2024-2025 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

{% for c in cases %}

/*---
desc: >-
    There are two cases of type override-compatibility, as types
    are used as either parameter types,
    or return types. For each case there are six kinds of types

    - Resizable Array type

    Every type is override-compatible with itself (see Invariance).
tags: [{{c.tags}}]
params: {{c.params}}
---*/

let res: number = 0;

class Base {
    {{c.base|indent}}
}

class Derived extends Base {
    {{c.derived|indent}}
}

function main() {
    {{(c.call or 'return')|indent}}
}
{% endfor %}
