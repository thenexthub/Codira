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

{% for c in cases %}

/*---
desc: >-
    A property can be implemented as a field in different combinations
---*/

interface I {
  f: {{c.check_type}}
}

class Base {
  f: {{c.check_type}} = {{c.base_value}}
}

class Derived extends Base implements I {
}

function get_prop(i: I): {{c.check_type}} {
  return i.f
}

function set_prop(i: I, value: {{c.check_type}}) {
  i.f = value
}

function main(): void {
  let a: Derived = new Derived()
  arktest.assertTrue(a instanceof Derived)
  arktest.assertTrue(a instanceof Base)
  arktest.assertTrue(a instanceof I)
  arktest.assertTrue(a.f instanceof {{c.check_type}})
  arktest.assertEQ(a.f, {{c.base_value}})
  arktest.assertEQ(get_prop(a), {{c.base_value}})
  set_prop(a, {{c.new_value}})
  arktest.assertEQ(a.f, {{c.new_value}})
  arktest.assertEQ(get_prop(a), {{c.new_value}})
}

{% endfor %}
