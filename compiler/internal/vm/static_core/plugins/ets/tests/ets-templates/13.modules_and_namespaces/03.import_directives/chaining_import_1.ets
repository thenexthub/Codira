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

/*---
desc: Chaining import (default -> selective -> all) Selective and all import the previously exported classes 
  with type keyword and extra export
files:
- ./chaining_import_0.ets
name: chaining_import_1
---*/

import type {DefaultExportedClass0 as SomeExportedClass1} from "./chaining_import_0"
import * as Test1 from "./chaining_import_0"

export const testvar: number = 1
export SomeExportedClass1

function main() {
    arktest.assertEQ(new Test1.TestClass().hello('hello'), 'hello_world')
}
