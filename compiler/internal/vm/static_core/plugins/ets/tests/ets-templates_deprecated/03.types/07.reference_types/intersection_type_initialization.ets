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
desc: For each Ti (1 ≤ i ≤ n), let Ci be the most specific class or array type such that Ti < Ci . Then there must be some Ck such that Ck < Ci for any i (1 ≤ i ≤ n), or a compile-time error occurs.
---*/

class A {}
final class B extends A {}

interface C {}
interface D extends C {}

final class F<T extends A, G extends C> {}

function main(): void {
  let instance = new F<B, D>();
}
