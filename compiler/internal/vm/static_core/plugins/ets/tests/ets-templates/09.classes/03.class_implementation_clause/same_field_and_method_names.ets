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

{% for type in class_types %}
{% for c in cases %}
/*---
desc: {{type}} {{c.doc}}
assert: >-
    A compile-time error occurs if a class field and a method from one of superinterfaces that a class implements
    have the same name, except when one is static and the other is not.
tags: []
---*/

interface IC {
  {{c.interface}}
}

{{type}} class CA implements IC {
  {{c.class}}
}

function main() {
}

{% endfor %}
{% endfor %}
