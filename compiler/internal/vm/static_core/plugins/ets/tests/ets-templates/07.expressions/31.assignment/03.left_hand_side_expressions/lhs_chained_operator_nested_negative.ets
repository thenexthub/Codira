/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
desc: Left-hand expression testing - the chaining operator, in field assignment.
tags: [compile-only, negative]
---*/

class Inner {
    value: number | null = 0;
}

class Outer {
    prop: Inner | null = new Inner();
}

function main(): void {
  let obj: Outer = new Outer();
  {{c}}
  arktest.assertEQ(obj.prop.value , 0);
}
{% endfor %}