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

{% for ic in integer_comparison %}

/*---
desc: Integer comparison {{ic.xvalue}} {{ic.op}} {{ic.yvalue}}
---*/

function main(): void {
  let x: {{ic.xtype}} = {{ic.xvalue}};
  let bx: {{ic.xalias}} = new {{ic.xalias}}({{ic.xvalue}});
  let y: {{ic.ytype}} = {{ic.yvalue}};
  let by: {{ic.yalias}} = new {{ic.yalias}}({{ic.yvalue}});
  arktest.assertEQ( (x {{ic.op}} y), {{ic.result}} )
  arktest.assertEQ( (bx {{ic.op}} by), {{ic.result}} )
  arktest.assertEQ( (x {{ic.op}} by), {{ic.result}} )
}

{% endfor %}
