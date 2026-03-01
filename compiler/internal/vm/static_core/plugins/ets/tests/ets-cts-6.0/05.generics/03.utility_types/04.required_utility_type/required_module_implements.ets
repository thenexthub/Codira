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
desc:  >-
    If class T has a user-defined getter, setter, or both, then none of those is called when object literal is used with
    Partial<T> variables. Object literal has its own built-in getters and setters to modify its variables.
    Use imported class which implements interface with {{c.field_type}} fields as T for Required<T>.
tags: [compile-only, negative]
files:
    - ../exported_data.ets
---*/

import {gs} from "../exported_data"
import {{c.import}} from "../exported_data"

function main() {
  let a: Required<{{c.use}}> = {{c.init}}
  a.i = 2.0
  a.j = "str2"
  let b = a.i
  let c = a.j
  arktest.assertEQ(gs,0, gs + " != 0")
}

{% endfor %}
