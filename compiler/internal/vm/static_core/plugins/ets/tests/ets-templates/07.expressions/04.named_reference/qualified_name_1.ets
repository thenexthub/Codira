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
desc: QualifiedName (see Names) is an expression that consists of dot-separated names.
files:
- ./named_reference_export4.ets
---*/

import {Shapes as alias} from "./named_reference_export4.ets"

class Clazz {
  // same name and namespace should not clash
  alias: alias.Square = new alias.Square;
}

class Shapes {}

function main() {

  let clazz = new Clazz;
  let s = new Shapes;
  
}
