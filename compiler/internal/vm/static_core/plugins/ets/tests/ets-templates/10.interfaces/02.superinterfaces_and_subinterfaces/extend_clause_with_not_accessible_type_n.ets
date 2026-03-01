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
desc: Declaring subinterface.
assert: Each typeReference in the extends clause of an interface declaration must name an accessible interface type (link to access control), or a compile-time error occurs.
tags: [compile-only, negative]
---*/

interface A {
  x: int = 1;
  
}

interface B {
  bar(): int;
}

interface C extends A, B, G {}
