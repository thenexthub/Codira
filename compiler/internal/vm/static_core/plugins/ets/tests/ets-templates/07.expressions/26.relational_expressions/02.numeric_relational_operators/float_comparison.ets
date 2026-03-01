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

{% for fc in float_comparison %}

/*---
desc: Integer comparison {{fc.xvalue}} {{fc.op}} {{fc.yvalue}}
---*/

function main(): void {
  let x: {{fc.xtype}} = {{fc.xvalue}}.to{{fc.xalias}}();
  let bx: {{fc.xalias}} = new {{fc.xalias}}(x);
  let y: {{fc.ytype}} = {{fc.yvalue}}.to{{fc.yalias}}();
  let by: {{fc.yalias}} = new {{fc.yalias}}(y);
  arktest.assertEQ( (x {{fc.op}} y), {{fc.result}} )
  arktest.assertEQ( (bx {{fc.op}} by), {{fc.result}} )
  arktest.assertEQ( (x {{fc.op}} by), {{fc.result}} )
}

{% endfor %}
