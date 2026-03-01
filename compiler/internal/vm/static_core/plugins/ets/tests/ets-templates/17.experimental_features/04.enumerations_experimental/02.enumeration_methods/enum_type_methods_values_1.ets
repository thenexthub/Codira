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
desc: values() returns an array of enumeration constants in the order of declaration.
---*/

enum Color { Red = "r1", Blue = "b1", Green = "g1" }

function f(a: FixedArray<Color>): number {
  return a.length;
}

function f(a: Object): int {
  return 2;
}

function main(): void {
  let colors = Color.values();
  arktest.assertEQ(colors[0], Color.Red);
  arktest.assertEQ(colors[1], Color.Blue);
  arktest.assertEQ(colors[2], Color.Green);
  arktest.assertEQ(f(colors), 3);
}
