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
desc: Field setter with static modifier
---*/

class A {
  private static _age: int = 0;

  static set age(value: int) {
    A._age = value;
  }

  static getAge(): int {
    return A._age;
  }
}

function main(): void {
  A.age = 5;
  arktest.assertEQ( A.getAge(), 5 );
}
