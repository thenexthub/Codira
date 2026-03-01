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
    Type string behaves like a reference type (see Reference Types) if it is created, assigned,
    or passed as an argument.
    A string object is immutable, the value of a string object cannot be changed after the object is created.
    The value of a string object can be shared.
---*/

function f(a: string) {
	a += "d"
	arktest.assertEQ(a, "abcd")
}

function main(): void {
  let a: string = "abc"
  f(a)
  arktest.assertEQ(a, "abc")
  arktest.assertNE(a, "abcd")
}
