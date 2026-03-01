/**
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
desc: Tuples are assignable to Object, Any.
---*/

function main() {
    let tuple: {{c.type}} = {{c.value}};
    let obj: Object = tuple;
    let a: Any = tuple;
    arktest.assertTrue(tuple instanceof Tuple)
    arktest.assertTrue(obj instanceof Object)
    arktest.assertTrue(a instanceof Any)
    arktest.assertEQ(tuple[0], {{c.expected_value}})
    arktest.assertEQ(obj[0], {{c.expected_value}})
    arktest.assertEQ(a[0], {{c.expected_value}})
}
{% endfor %}
