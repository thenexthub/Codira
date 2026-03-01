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
desc: ArkTS supports the following two overloading mechanisms overload signatures and overload declarations. Legacy overload must be removed.
tags: [compile-only, negative]
---*/

function foo<T>(a: T): int {
  return 1;
}
function foo<T, V>(a: T, b: V): int {
  return 2;
}
function foo<T, U, V>(a: T, b: U, c: V): int {
  return 3;
}

function main(): void {
  let v: string = foo<string, string>(v, v);
}
