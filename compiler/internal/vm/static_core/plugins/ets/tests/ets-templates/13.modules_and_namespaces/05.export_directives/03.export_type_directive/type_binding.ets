/*---
Copyright (c) 2025 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: >-
      An export directive can have a type modifier exclusively for a better syntactic compatibility with TypeScript
      (also see Import Type Directive).
files:
- ./export_type.ets
---*/

import type {D} from "./export_type"

function main() {
  let a: D = new D()
  arktest.assertEQ(a.f, 8)
}
