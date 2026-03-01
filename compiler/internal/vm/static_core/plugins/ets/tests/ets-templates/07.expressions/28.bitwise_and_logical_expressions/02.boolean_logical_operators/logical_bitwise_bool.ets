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

{% for i in logical_ops %}

/*---
desc: Bitwise logical operations (bool)
params: {{i.xorigin}} {{i.op}} {{i.yorigin}} == {{i.dest}}
---*/

function main(): void {
  let x: boolean = {{i.xorigin}};
  let y: boolean = {{i.yorigin}};
  let dest: boolean = {{i.dest}};
  arktest.assertEQ(x {{i.op}} y, dest);

  const a: boolean = {{i.xorigin}};
  const b: boolean = {{i.yorigin}};
  arktest.assertEQ(a {{i.op}} b, dest);

  arktest.assertEQ({{i.xorigin}} {{i.op}} {{i.yorigin}}, dest);
}

{% endfor %}
