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
{% for d in c['decl'] %}
/*---
desc: >-
    The type of an array literal can be inferred from the declaration context.
tags: [{{c.tags}}]
---*/
{{c.defs}}
let a: {{d[0]}}

function main() {
    {{c.use|indent}}
    {% if d[1] %}let x = a as {{d[1]}}{% endif %}
    {{c.check|indent}}
}
{% endfor %}
{% endfor %}
