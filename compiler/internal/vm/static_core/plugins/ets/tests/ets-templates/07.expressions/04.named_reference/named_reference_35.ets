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
desc: Incorrect type of generic function parameters.
---*/

interface Lengthwise {
  length: number;
}

function logLength<T extends Lengthwise>(obj: T): void {
  let x = obj.length;
  arktest.assertEQ(x, 5);
}

function main(): void {
  const obj: Lengthwise = {length: 5};
  logLength(obj);
}