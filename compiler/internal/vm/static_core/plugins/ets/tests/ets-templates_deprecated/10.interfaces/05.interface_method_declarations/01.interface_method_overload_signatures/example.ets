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
      The order of textual declarations of overloaded signatures affects the manner
      a type check is performed at the call site, starting from the signature
      declared first and then proceeding in the proper sequence.
---*/

interface I {
  foo(): number // 1st signature
  foo(p: string): string // 2nd signature
  foo(p1: string, p2?: number): number // 3rd signature
}

final class C implements I {
  foo(...p: Any[]): Any {
    if(p.length == 0)
      return 7
    else if (p.length == 1)
      return (p + "str")
    else
      return 9
  }
}

function demo(i: I) {
  arktest.assertEQ(i.foo(), 7)
  arktest.assertEQ(i.foo("aa"), "aa str")
  arktest.assertEQ(i.foo("aa", undefined), 9)
}

function main() {
  demo(new C())
}
