/*---
Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
      Another form of type import is used when type is attached to a name binding. This allows mixing general import
      and type import.
files:
- ./export_type_with_bind.ets
---*/

import {Class1, type Class2 } from "./export_type_with_bind"

function main(): void {
    let c1 = new Class1();
    let c2 = new Class2();
    arktest.assertEQ(c1.f, 1);
    arktest.assertEQ(c2.f, 2);
}
