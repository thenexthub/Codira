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

class Base implements I {
  f: {{c.check_type}} = {{c.implement_value}}
}

class Derived extends Base {
  override f: {{c.check_type}} = {{c.override_value}}
}

function get_prop(i: I): {{c.check_type}} {
  return i.f
}

function set_prop(i: I, value: {{c.check_type}}) {
  i.f = value
}

function get_field(a: Base): {{c.check_type}} {
  return a.f
}

function set_field(a: Base, value: {{c.check_type}}) {
  a.f = value
}

function main(): void {
  let a: Derived = new Derived()
  arktest.assertTrue(a instanceof Derived)
  arktest.assertTrue(a instanceof Base)
  arktest.assertTrue(a instanceof I)
  arktest.assertTrue(a.f instanceof {{c.check_type}})
  arktest.assertEQ(a.f, {{c.override_value}})
  arktest.assertEQ(get_prop(a), {{c.override_value}})
  arktest.assertEQ(get_field(a), {{c.override_value}})
  set_field(a, {{c.new_value}})
  arktest.assertEQ(a.f, {{c.new_value}})
  arktest.assertEQ(get_prop(a), {{c.new_value}})
  arktest.assertEQ(get_field(a), {{c.new_value}})
  set_prop(a, {{c.override_value}})
  arktest.assertEQ(a.f, {{c.override_value}})
  arktest.assertEQ(get_prop(a), {{c.override_value}})
  arktest.assertEQ(get_field(a), {{c.override_value}})
}

{% endfor %}
