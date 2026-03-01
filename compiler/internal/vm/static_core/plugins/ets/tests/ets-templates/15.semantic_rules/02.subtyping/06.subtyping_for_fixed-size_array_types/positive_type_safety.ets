/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
 */

{%- for c in cases %}

/*---
desc: >-
    Union of string literals in FixedArray erased to FixedArray<String> in runtime
---*/

let x: FixedArray<{{c.stype}}> = {{c.value}}
let y: FixedArray<{{c.dtype}}> = x

function foo(ca: FixedArray<{{c.dtype}}>) {
  ca[0] = {{c.newValue}}
}

function main() {
    foo(x)
    foo(y)
    arktest.assertEQ(x[0], {{c.newValue}})
    arktest.assertEQ(y[0], {{c.newValue}})
}

{%- endfor %}
