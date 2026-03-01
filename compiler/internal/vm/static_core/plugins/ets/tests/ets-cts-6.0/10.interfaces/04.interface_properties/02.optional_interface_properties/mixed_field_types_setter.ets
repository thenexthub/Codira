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
desc: >-
    If the default setter is not overridden in a class that implements the interface, InvalidStoreAccessError is
    thrown at attempt to set value of an optional property.
    Class declares mandatory Interface fields, some optional fields and has own fields.
---*/

interface I {
  a?: number
  b?: number
  c: number
}

class C implements I {
  a: number|undefined = 2
  c: number = 1
  d: number = 3
  e?: number = undefined
}

function main() {
  let res = false
  let i: C = new C()
  try {
    i.b = 2
  } catch (e) {
    arktest.assertEQ(i.b, undefined)
    arktest.assertTrue(e instanceof InvalidStoreAccessError)
    res = true
  }
  arktest.assertTrue(res)
}
