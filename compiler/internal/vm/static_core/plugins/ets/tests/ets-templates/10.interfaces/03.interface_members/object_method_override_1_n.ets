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
desc: Override Object's methods in interface with wrong throws clause type.
assert: It is a compile-time error if the interface explicitly declares a method with a signature that is override-equivalent (link to signatures) to a public method of Object, but which has a different return type, or an incompatible throws clause, or is not abstract.
tags: [compile-only, negative]
---*/

interface A {
  toString(): int throws;
}

class B implements A {
  
}
