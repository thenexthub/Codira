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
  The scope of a type parameter name in a class or interface declaration is
  that entire declaration, excluding static member declarations.
tags: [compile-only, negative]
expected_out: "{{c.expected_out}}"
---*/

class C<T extends Number> {
  {{c.decl}}
}

{% endfor %}