/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
 desc: 03 Types
 name: issue15013_01.ets
 files: ['./issue15013_to_export.ets']
 tags:
 ---*/

import {TestNullishType} from './issue15013_to_export'

function foo<T>(x: T): TestNullishType {
  return x as Object | null | undefined
}

function main(): void {
  let y = foo(new Object())
  arktest.assertTrue( y instanceof Object)
}
