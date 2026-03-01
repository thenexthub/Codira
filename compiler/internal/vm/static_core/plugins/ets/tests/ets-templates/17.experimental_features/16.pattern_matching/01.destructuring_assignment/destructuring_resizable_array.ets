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

{%- for c in cases %}

/*---
desc: Destructuring Assignment - Resizable Arrays
---*/

function resizable_array(): number[] {
  return [1,2,3,4,5];
}

function main() {
    let n1: number = 0;
    let n2: number = 0;
    let n3: number = 0;
    let n4: number = 0;
    let n5: number = 0;
    let n6: number = 0;

    {{c.use|indent}}
}

{%- endfor %}
