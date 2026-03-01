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
    The supertype for string literals (see String Literals) is type string.
    This affects the overriding -
    {{c.doc}}
tags: [{{c.tags}}]
---*/

{{c.decl|indent}}

{% if "negative" not in c.tags %}
function main() {
    {{c.use|indent}}
}
{% endif %}
{% endfor %}
