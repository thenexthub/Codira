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
desc: Test power operator ** with Long - {{c.description}}
---*/

function main() {
    let a: long = {{c.val1}};
    let b: long = {{c.val2}};
    let result = a ** b;
    
    // Long should remain long, no casting to other types
    arktest.assertTrue(result instanceof long);
    arktest.assertEQ(result, {{c.expected}});
}
{%- endfor %}
