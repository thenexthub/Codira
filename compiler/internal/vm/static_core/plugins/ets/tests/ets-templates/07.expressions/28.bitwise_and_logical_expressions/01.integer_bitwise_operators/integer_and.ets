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

{% for i in integer_and %}

/*---
desc: Integer 'and' operation
params: {{i.xorigin}} {{i.op}} {{i.yorigin}} == {{i.dest}} ({{i.xbits}} {{i.op}} {{i.ybits}} == {{i.dbits}})
---*/

function main(): void {
  let x: byte = {{i.xorigin}};
  let y: byte = {{i.yorigin}};
  let dest: byte = {{i.dest}};
  arktest.assertTrue((x {{i.op}} y) == dest);
}

{% endfor %}
