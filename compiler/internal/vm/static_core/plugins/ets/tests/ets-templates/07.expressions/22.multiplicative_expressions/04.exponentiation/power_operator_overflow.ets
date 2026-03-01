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
desc: Test power operator ** overflow behavior with {{c.type1}} and {{c.type2}}
---*/

function main() {
    let a: {{c.type1}} = {{c.val1}};
    let b: {{c.type2}} = {{c.val2}};
    let result = a ** b;
    
    // For byte/short operations, result should always be int (no overflow)
    arktest.assertTrue(result instanceof int);

    // For int/long, check the actual overflow result
    arktest.assertEQ(result, {{c.expected}});
}
{%- endfor %}
