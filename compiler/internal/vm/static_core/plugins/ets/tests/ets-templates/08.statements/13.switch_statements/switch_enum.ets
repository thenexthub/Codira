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
desc: Switch statement with enum
---*/

enum Color { Red, Yellow, Green }

function f(i: Color): void {
  switch (i) {
    case Color.Red:    arktest.assertEQ(i, Color.Red);    break;
    case Color.Yellow: arktest.assertEQ(i, Color.Yellow); break;
    case Color.Green:  arktest.assertEQ(i, Color.Green);  break;
  }
}

function main(): void {
  f(Color.Red);
  f(Color.Yellow);
  f(Color.Green);
}
