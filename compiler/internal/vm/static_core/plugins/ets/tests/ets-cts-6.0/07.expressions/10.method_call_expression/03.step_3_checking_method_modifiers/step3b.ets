/*---
Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

{%- for c in cases %}
/*---
desc: >-
    If the method call has the form expression.identifier, then
    the method must not be declared static; a compile-time error occurs otherwise.
---*/

{{c.decl}}

function main() {
    {{c.use|indent}}
}
{%- endfor %}
