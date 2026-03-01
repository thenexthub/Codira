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
 desc: 13.03 Import Directives
 name: issue14991_01
 tags:
 files: ['./issue14991_03.ets', './issue14991_02.ets']
 ---*/

import { A } from './issue14991_02'
import { I } from './issue14991_03'

class B implements I {
  foo(): string {
    return "B implements I";
  }
}

function main() {
  const a: A = new A();
  arktest.assertEQ(a.foo(), "A implements I")
}
