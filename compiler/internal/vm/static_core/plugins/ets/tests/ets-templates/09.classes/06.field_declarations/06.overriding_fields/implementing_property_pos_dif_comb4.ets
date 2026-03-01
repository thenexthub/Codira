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

interface I1 {
  f: {{c.check_type}}
}
interface I2 {
  f: {{c.check_type}}
}
interface I3 extends I2 {
}
class A implements I1, I3 {
  f: {{c.check_type}} = {{c.base_value}}
}

function get_prop_I1(i: I1): {{c.check_type}} {
  return i.f
}

function set_prop_I1(i: I1, value: {{c.check_type}}) {
  i.f = value
}

function get_prop_I2(i: I2): {{c.check_type}} {
  return i.f
}

function set_prop_I2(i: I2, value: {{c.check_type}}) {
  i.f = value
}

function get_prop_I3(i: I3): {{c.check_type}} {
  return i.f
}

function set_prop_I3(i: I3, value: {{c.check_type}}) {
  i.f = value
}

function main(): void {
  let a: A = new A()
  arktest.assertTrue(a instanceof A)
  arktest.assertTrue(a instanceof I1)
  arktest.assertTrue(a instanceof I2)
  arktest.assertTrue(a.f instanceof {{c.check_type}})
  arktest.assertEQ(a.f, {{c.base_value}})
  arktest.assertEQ(get_prop_I1(a), {{c.base_value}})
  arktest.assertEQ(get_prop_I2(a), {{c.base_value}})
  arktest.assertEQ(get_prop_I3(a), {{c.base_value}})
  set_prop_I1(a, {{c.new_value}})
  arktest.assertEQ(a.f, {{c.new_value}})
  arktest.assertEQ(get_prop_I1(a), {{c.new_value}})
  arktest.assertEQ(get_prop_I2(a), {{c.new_value}})
  arktest.assertEQ(get_prop_I3(a), {{c.new_value}})
  set_prop_I2(a, {{c.base_value}})
  arktest.assertEQ(a.f, {{c.base_value}})
  arktest.assertEQ(get_prop_I1(a), {{c.base_value}})
  arktest.assertEQ(get_prop_I2(a), {{c.base_value}})
  arktest.assertEQ(get_prop_I3(a), {{c.base_value}})
  set_prop_I3(a, {{c.new_value}})
  arktest.assertEQ(a.f, {{c.new_value}})
  arktest.assertEQ(get_prop_I1(a), {{c.new_value}})
  arktest.assertEQ(get_prop_I2(a), {{c.new_value}})
  arktest.assertEQ(get_prop_I3(a), {{c.new_value}})
}

{% endfor %}
