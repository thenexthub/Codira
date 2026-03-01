/*---
Copyright (c) 2026 Huawei Device Co., Ltd.
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
    Getting overridden field through function
---*/

class Base {
  field: number = 5
}

class A extends Base {
  field: number = 7
}

function f(v: Base) {
  arktest.assertTrue(v.field instanceof number)
  arktest.assertEQ(v.field, 7)
}

function main(): void {
  f(new A)
}
