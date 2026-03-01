/*---
* Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

/*---
desc: >-
    If type parameter T has type constraint S, then the actual type of the generic instantiation must be
    compatible with S (see Type Compatibility).
tags: [compile-only, negative]
---*/

{{c.decl}}

function foo<X extends {{c.type}}>(p: X): X { return p };

function main(): void {
  foo<{{c.use}}>({{c.p}})
}

{% endfor %}
