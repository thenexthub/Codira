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

{% for ic in literals_comparison %}

/*---
desc: comparison with literals - {{ic.type}}
---*/

function main(): void {
    const one: int = 1
    {% if ic.type != 'NaN' %}
    arktest.assertLT(one, {{ic.value}})
    arktest.assertLT(-{{ic.value}}, one)
    arktest.assertLT(one, {{ic.valueWithUndercore}})
    arktest.assertLT(-{{ic.valueWithUndercore}}, one)
    {% else %}
    arktest.assertFalse(one > {{ic.value}})
    arktest.assertFalse(one < {{ic.value}})
    arktest.assertFalse(one > -{{ic.value}})
    arktest.assertFalse(one < -{{ic.value}})
    {% endif %}
}

{% endfor %}
