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
   If a function type has the ‘?’ mark for a parameter name, then this parameter and all parameters
   that follow (if any) are optional. Otherwise, a compile-time error occurs... This parameter has
   no default value.
tags: [compile-only, negative]
---*/

{{c.decl}}

function main() {
     arktest.assertTrue( false, 'unreachable code, CTE expected' )
}

{%- endfor %}
