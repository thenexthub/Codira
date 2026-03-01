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
desc: Accessing the length property returns the string length as int type value
---*/

function main(): void {
  let a: string = "012 \n3456789\n ABCDEFGHIJKLMNOPQRSTUVWXYZ\u0000\u03e9\u07d2\u0bbb\u0fa4\u138d\u1776\u1b5f\u1f48\u2331\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19"
  arktest.assertEQ(a.length, 60)
  arktest.assertTrue(a.length instanceof int)
}
