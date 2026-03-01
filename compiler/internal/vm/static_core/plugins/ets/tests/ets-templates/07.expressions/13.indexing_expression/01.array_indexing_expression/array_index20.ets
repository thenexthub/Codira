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

/*---
desc: Test the array index modification of fields when array elements are reference types.
---*/

class Container {
  public value: number;
}

function main(): void {
  let containers: Container[] = [new Container(), new Container()];
  containers[0].value = 10;
  arktest.assertEQ(containers[1].value, 0);

  let first = containers[0];
  first.value = 20;
  arktest.assertEQ(containers[0].value, 20);
}