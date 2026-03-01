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
    Overriding interface property
---*/

interface I {
  f: {{c.check_type}}
}

interface II extends I {
  f: {{c.check_type}}
}

class A implements II {
  f: {{c.check_type}} = {{c.implement_value_1}}
}

class B implements I {
  f: {{c.check_type}} = {{c.implement_value_2}}
}

function get_prop_I(i: I): {{c.check_type}} {
  return i.f
}

function set_prop_I(i: I, value: {{c.check_type}}) {
  i.f = value
}

function get_prop_II(i: I): {{c.check_type}} {
  return i.f
}

function set_prop_II(i: I, value: {{c.check_type}}) {
  i.f = value
}

function main(): void {
  let a: A = new A()
  let b: B = new B()
  arktest.assertTrue(a instanceof A)
  arktest.assertTrue(a instanceof II)
  arktest.assertTrue(a instanceof I)
  arktest.assertTrue(a.f instanceof {{c.check_type}})
  arktest.assertEQ(a.f, {{c.implement_value_1}})
  arktest.assertEQ(get_prop_II(a), {{c.implement_value_1}})
  arktest.assertEQ(get_prop_I(a), {{c.implement_value_1}})
  set_prop_II(a, {{c.new_value}})
  arktest.assertEQ(a.f, {{c.new_value}})
  arktest.assertEQ(get_prop_II(a), {{c.new_value}})
  arktest.assertEQ(get_prop_I(a), {{c.new_value}})
  set_prop_I(a, {{c.implement_value_1}})
  arktest.assertEQ(a.f, {{c.implement_value_1}})
  arktest.assertEQ(get_prop_II(a), {{c.implement_value_1}})
  arktest.assertEQ(get_prop_I(a), {{c.implement_value_1}})
  arktest.assertTrue(b instanceof B)
  arktest.assertTrue(b instanceof I)
  arktest.assertTrue(b.f instanceof {{c.check_type}})
  arktest.assertEQ(b.f, {{c.implement_value_2}})
  arktest.assertEQ(get_prop_I(b), {{c.implement_value_2}})
  set_prop_I(b, {{c.new_value}})
  arktest.assertEQ(b.f, {{c.new_value}})
  arktest.assertEQ(get_prop_I(b), {{c.new_value}})
}

{% endfor %}
