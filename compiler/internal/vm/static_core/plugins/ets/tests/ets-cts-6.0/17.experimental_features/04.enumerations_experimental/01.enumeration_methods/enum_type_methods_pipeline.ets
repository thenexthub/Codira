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
desc: |-
    Method getName() returns the name of an enumeration constant.
    Method valueOf() returns an int or string value of an enumeration constant
    depending on the type of the enumeration constant.
---*/

enum Color {Red = 777, Blue = 55, Green = 2}

function main(): void {
  arktest.assertTrue( Color.Red.getName() == "Red" );
  arktest.assertTrue( Color.Red.valueOf() == 777 );
}
