/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
desc: >-
  If the type of an array literal [ expr1, ... , exprN ] cannot be inferred from the context,
  then it is inferred from the initialization expressions by using the following algorithm.
  The array literal type is an array of elements of the union type.
---*/

{{c.decl}}

function main() {
    {{c.use|indent}}
}
{% endfor %}
