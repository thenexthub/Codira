/*---
Copyright (c) 2025 Huawei Device Co., Ltd.
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
desc: Switch statement
---*/


let pd: int = 0;
let last: int = 0;

function f(i: int): void {
  switch (i) {
    case 0: { arktest.assertEQ( i, 0); break; }
    case 1: { arktest.assertEQ( i, 1); break; }
    default: { pd++; break; }
    case 2: { last++; break; }

  }
}

function main(): void {
  pd = 0;
  last = 0;
  f(2);

  arktest.assertEQ(0, pd)
  arktest.assertEQ(1, last)
}
