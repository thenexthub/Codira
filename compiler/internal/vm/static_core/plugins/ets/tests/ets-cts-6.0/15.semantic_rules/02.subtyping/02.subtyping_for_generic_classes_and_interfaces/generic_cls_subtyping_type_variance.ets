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
desc: Subtyping relation between generic classes with type variance
---*/

class D {}
class E extends D {}
class F extends E {}

class A<in T, out S> {
  readonly s: S
  constructor(s: S) { this.s = s }
  foo(t: T): S { return this.s }
}

function main(): void {
  let base = new A<{{case.base_type1}},{{case.base_type2}}>(new {{case.base_constr}})
  let subtype = new A<{{case.sub_type1}},{{case.sub_type2}}>(new {{case.sub_constr}})
  arktest.assertTrue(base.foo(new {{case.base_call}}) instanceof {{case.expected_type_base}})
  base = subtype
  arktest.assertTrue(base.foo(new {{case.sub_call}}) instanceof {{case.expected_type_base}})
  arktest.assertTrue(base.foo(new {{case.sub_call}}) instanceof {{case.expected_type_sub}})
}

{% endfor %}
