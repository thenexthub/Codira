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

{% for case in cases %}

/*---
desc: Subtyping relation between generic classes and interfaces with type variance
tags: [negative, compile-only]
---*/

class D {
}
class E extends D {
}
class F extends E {
}

interface A<in T, out S> {
  t: T
  s: S
}

function main(): void {
  let base: A<{{case.first_type}}, {{case.second_type}}> = { t: {}, s: {} }
  let subtype: A<{{case.first_type2 if case.first_type2 else case.second_type}},
                 {{case.second_type2 if case.second_type2 else case.first_type}}> = { t: {}, s: {} }
  arktest.assertTrue(base.t instanceof {{case.first_type}})
  arktest.assertTrue(base.s instanceof {{case.second_type}})
  base = subtype
  arktest.assertTrue(base.t instanceof {{case.first_type2 if case.first_type2 else case.second_type}})
  arktest.assertTrue(base.s instanceof {{case.second_type2 if case.second_type2 else case.first_type}})
}

{% endfor %}
