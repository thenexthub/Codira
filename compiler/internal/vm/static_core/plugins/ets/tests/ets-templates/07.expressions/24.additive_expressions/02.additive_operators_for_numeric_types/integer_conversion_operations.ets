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

{% for op in operations %}

/*---
desc: Additive operations with implicit type conversion
params: {{op.value1}}({{op.type1}}) {{op.operation}} {{op.value2}}({{op.type2}}) == 
        {{op.result_value}}({{op.result_type}})
---*/

function main(): void {
  let x: {{op.type1}} = {{op.value1}};
  let y: {{op.type2}} = {{op.value2}};
  let z = x {{op.operation}} y;
  arktest.assertTrue(z instanceof {{op.result_type}}, "Result type isn't " + "{{op.result_type}}");
  arktest.assertEQ(z, {{op.result_value}})
}
{% endfor %}
