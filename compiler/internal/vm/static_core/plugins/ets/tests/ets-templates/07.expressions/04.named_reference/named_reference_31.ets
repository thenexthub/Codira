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
desc: The qualified name is incorrect, the interface is referenced. A compilation error is reported.
files:
- ./named_reference_export6.ets
- ./named_reference_export7.ets
tags:
- negative
- compile-only
---*/

import {ShapeInterfaces} from './named_reference_export6.ets';
import {Circle} from './named_reference_export7.ets';

function main(): void {
  const calculableShape: ShapeInterface.Calculable = new Circle(5);
}