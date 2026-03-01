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
    Field marked with the modifier override does not override a field from a superclass.
tags: [compile-only, negative]
expected_out: "Semantic error ESE49702: The 'override' modifier is invalid here because the base class 'Base' does not declare this member."
---*/

class Base {
  field1: number = 0
}

class A extends Base {
  override field2: number = 5
}

function main(): void {
}
