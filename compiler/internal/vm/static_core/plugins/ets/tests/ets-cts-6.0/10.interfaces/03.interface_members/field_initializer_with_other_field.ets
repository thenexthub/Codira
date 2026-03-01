/*---
Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
desc: Field initializer with reference to another interface field.
assert: >-
    It is a compile-time error if the initializer of an interface field uses another field
    whose declaration occurs textually later in the same interface.
tags: [compile-only, negative]
---*/

interface A {
  x: int = y + 5;
  y: int = 5;
}
