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
desc: >-
    The undefined type can be used as the type argument that instantiates a generic type if specific value
    of the type argument is irrelevant.
---*/

class A<T> {
  foo(): int {
    return 1;
  }
}

function main(): void {
  let i: A<undefined> = new A<undefined>;
  arktest.assertTrue( i.foo() == 1 )
}
