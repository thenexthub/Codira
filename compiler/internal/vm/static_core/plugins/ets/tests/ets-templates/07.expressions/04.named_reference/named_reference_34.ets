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
desc: Block-level scope has higher priority than global scope.
---*/

let count = 10;

function testBlockScope(): void {
  if (true) {
    let count = 20;
    arktest.assertEQ(count, 20);
  }
  arktest.assertEQ(count, 10);
}

function main(): void {
  testBlockScope();
}