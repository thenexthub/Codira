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
desc: Switch statement with char
---*/

function f(i: char): void {
  switch (i) {
    case c'0':
      arktest.assertEQ( i, c'0' )
      break;
    case c'1':
      arktest.assertEQ( i, c'1' )
      break;
    case c'2':
      arktest.assertEQ( i, c'2' )
      break;
    default:
      arktest.assertTrue( false )
  }
}

function main(): void {
  f(c'0');
  f(c'1');
  f(c'2');
}
