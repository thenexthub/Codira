/*---
Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
desc: Type T is not compatible (see Type Compatibility) with Required<T>
tags: [compile-only, negative]
---*/

abstract class A {
  i: number = 0 ;
  j: string = "";
}

function main() {
  let a: Required<A> = {i: 1.9, j: "some str"}
  let b: A = a
}
