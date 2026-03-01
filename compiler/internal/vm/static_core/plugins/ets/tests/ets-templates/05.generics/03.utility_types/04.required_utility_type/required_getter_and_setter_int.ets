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
desc:  >-
    No method (including any getter or setter) of T is part of the Required<T> type.
    Note. If class T has a user-defined getter, setter, or both, then none of those is called when object literal is used with
    Partial<T> variables. Object literal has its own built-in getters and setters to modify its variables.
---*/

interface I {
  set i(i: number|undefined)
  get i(): number|undefined
}

function main() {
  let a: Required<I> = {i: 1.9}
  a.i = 1.9
  arktest.assertEQ( a.i, 1.9 )
}
