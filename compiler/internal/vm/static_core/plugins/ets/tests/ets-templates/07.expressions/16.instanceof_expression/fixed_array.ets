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

{% for c in cases %}
/*---
desc: |-
    A compile-time error occurs if type T is not retained by Type Erasure... Instantiated generic types
    cannot be used because the T operand of an instanceof must be retained by Type Erasure...
    Original Type Fixed-Size Array Types (FixedArray<T>) Effective Type Instantiations of FixedArray<T>
    (i.e., the effective type of type argument T is preserved).
---*/

// Note above docs are quoted from spec chapters "7.16 InstanceOf Expression" and "15.12 Type Erasure"

{{c.decl}}
class A {}

function main() {
    {{c.use}}
}
{% endfor %}
