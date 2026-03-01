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
desc: >-
    getValueOf(string) returns an enumeration constant with the given name,
    or throws an error if no constant with such name exists.
---*/

enum Color {Red, Green, Blue}

function main(): void {
  let c: Color = Color.Red;
  let colors = Color.values();
  let red = Color.getValueOf("Red");
  arktest.assertEQ( red, colors[0] );
  arktest.assertTrue(c == red);
}
