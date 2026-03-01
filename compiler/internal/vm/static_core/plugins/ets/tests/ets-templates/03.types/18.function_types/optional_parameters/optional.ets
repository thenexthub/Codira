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

{%- for c in cases %}

/*---
desc: >-
    Function type can be used to express the expected signature of a function. A function type consists
    of the following: Optional type parameters... A type alias can set a name for a function type...
    The actual type of the parameter is... a union of the parameter type and type undefined.
---*/

{{c.decl}}

function main() {
    {{c.use|indent}}
}

{%- endfor %}
