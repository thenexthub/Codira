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
expected_out: "Semantic error ESE46316: The 'override' modifier is invalid here because class 'A' does not extends another class."
---*/

interface I {
  field: int
}
class A implements I {
  get field() { return 1 }
  set field(f: int) {}
  override field1: int
}

function main(): void {
}
