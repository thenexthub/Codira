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

{% for ni in nan_inf %}

/*---
desc: Division with INF
params: {{ni.x}} / {{ni.y}} == {{ni.result}}
---*/

function main(): void {
  let lhs = {{ni.x}}
  let rhs = {{ni.y}}
  let res = lhs / rhs
  arktest.assertEQ( res, {{ni.result}} );
}
{% endfor %}
