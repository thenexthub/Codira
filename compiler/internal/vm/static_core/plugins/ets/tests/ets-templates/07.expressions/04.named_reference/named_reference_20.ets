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
desc: To verify the initialization behavior of default parameter values.
---*/

let defaultVal = 5 as number;

function withDefault(param: number = defaultVal + 1): number {
  return param;
}

function main(): void {
  let x: number = withDefault();
  arktest.assertEQ(x, 6);

  let y: number = withDefault(10);
  arktest.assertEQ(y, 10);
}