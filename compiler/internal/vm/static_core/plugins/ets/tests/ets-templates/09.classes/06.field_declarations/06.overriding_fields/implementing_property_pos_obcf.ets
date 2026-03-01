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
    Providing implementation for the interface property
    in the form of a field with overriding base class field
---*/

class C {
  f: {{c.check_type}} = {{c.base_value}}
}

interface I {
  f: {{c.check_type}}
}

class D extends C implements I {
  f: {{c.check_type}} = {{c.override_value}}
}

function get_prop(i: I): {{c.check_type}} {
  return i.f
}

function set_prop(i: I, value: {{c.check_type}}) {
  i.f = value
}

function get_field(a: C): {{c.check_type}} {
  return a.f
}

function set_field(a: C, value: {{c.check_type}}) {
  a.f = value
}

function main(): void {
  let a: D = new D()
  arktest.assertTrue(a instanceof D)
  arktest.assertTrue(a instanceof C)
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
