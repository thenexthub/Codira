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
    Overriding in first and second derived class
---*/

class C {
  field: {{c.check_type}}
  constructor() {
    this.field = {{c.base_value}}
  }
}

class D extends C {
  override field: {{c.check_type}} = {{c.override_value_1}}
}

class Derived extends D {
  field: {{c.check_type}}
  constructor() {
    super()
    this.field = {{c.override_value_2}}
  }
}

function get_field(a: C): {{c.check_type}} {
  return a.field
}

function set_field(a: C, value: {{c.check_type}}) {
  a.field = value
}

function main(): void {
  let a: Derived = new Derived()
  let b: D = new D()
  arktest.assertTrue(a.field instanceof {{c.check_type}})
  arktest.assertEQ(a.field, {{c.override_value_2}})
  arktest.assertEQ(get_field(a), {{c.override_value_2}})
  set_field(a, {{c.new_value}})
  arktest.assertEQ(a.field, {{c.new_value}})
  arktest.assertEQ(get_field(a), {{c.new_value}})
  arktest.assertTrue(b.field instanceof {{c.check_type}})
  arktest.assertEQ(b.field, {{c.override_value_1}})
  arktest.assertEQ(get_field(b), {{c.override_value_1}})
  set_field(b, {{c.new_value}})
  arktest.assertEQ(b.field, {{c.new_value}})
  arktest.assertEQ(get_field(b), {{c.new_value}})
}

{% endfor %}
