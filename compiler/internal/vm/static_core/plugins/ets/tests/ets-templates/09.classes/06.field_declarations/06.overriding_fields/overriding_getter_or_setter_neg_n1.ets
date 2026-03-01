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
    Field overrides getter or setter in a superclass
tags: [compile-only, negative]
expected_out: "Semantic error ESE129829: 'num' is defined as an accessor in class 'C', but is overridden here in 'D' as an instance property."
---*/

class C {
  get num(): number { return 42 }
  set num(x: number) {}
}
class D extends C {
  num: number = 2
}

function main(): void {
}
