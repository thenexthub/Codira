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
desc: Restricting access Interface by namespace, object that does not meet the interface definition.
files:
- ./named_reference_export6.ets
tags:
- negative
- compile-only
---*/

import {ShapeInterfaces} from './named_reference_export6.ets';

function main(): void {
  const invalid: ShapeInterfaces.Calculable = {Calculable: "not a function"};
}