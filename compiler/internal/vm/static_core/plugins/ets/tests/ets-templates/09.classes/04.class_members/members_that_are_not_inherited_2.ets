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
desc: Constructors and class initializers are not members and are not inherited.
---*/

let iteration_number = 0;

class CA {
  static f: int = 1;
  static {
    CA.f += 2;
  }
}

class CB extends CA {
  static {
    CA.f += 4;
  }
}

function main(): void {
  if (iteration_number < 1) {
    arktest.assertEQ(CA.f, 3);
  } else {
    arktest.assertEQ(CA.f, 7);
  }
  let a = new CB();
  arktest.assertEQ(CA.f, 7);
  iteration_number += 1;
}
