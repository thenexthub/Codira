/*---
Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
    As a result of assignment to a variable or call, the actual runtime type of a parameter
    of class or interface can become different from the type explicitly specified
    or inferred at the point of declaration.
    In this situation method calls are dispatched during program execution based on their actual type
    {{c.desc or ""}}
---*/

{{c.decl}}

function main() {
  {{c.use}}
}

{% endfor %}
