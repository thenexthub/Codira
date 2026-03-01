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

{% for inb in unary_minus_inbound %}

/*---
desc: Unary minus operator (inbound)
params: {{inb.type}} {{inb.origin}} => {{inb.dest}}
---*/

function main(): void {
  let a: {{inb.type}} = {{inb.origin}};
  arktest.assertEQ(-a, {{inb.dest}});
}

{% endfor %}
