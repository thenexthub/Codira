/*---
Copyright (c) 2026 Huawei Device Co., Ltd.
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
    A compile-time error occurs if a field is not declared as readonly
    in a superclass, while an overriding field is marked as readonly
tags: [compile-only, negative]
expected_out: "Expected CTE output is unknown"
---*/

class C {
  field = 1
}
class D extends C {
  readonly field = 2
}

function main(): void {
}
