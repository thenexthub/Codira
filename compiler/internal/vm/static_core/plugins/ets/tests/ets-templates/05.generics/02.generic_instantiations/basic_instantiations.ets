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

{% for c in cases %}
/*---
desc: |-
   A generic instantiation G < T 1 , ..., T n > is well-formed if all of the following is true:
    * The generic declaration name is G.
    * The number of type arguments equals that of G’s type parameters.
    * All type arguments are subtypes of a corresponding type parameter constraint.
tags: [compile-only]
params: c
---*/

class SomeType {}
interface OtherType {}

{{c.decl|indent}}

function main(): void {
    {{c.use|indent}}
}

{% endfor %}
