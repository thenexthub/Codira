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
    When extending a class, an instance field declared in a superclass
    can be overridden by a same-name, same-type field.
    The new declaration adds no new field to the class type but allows
    changing field initialization.
params: overriding in second derived class with override keyword
---*/

class Base {
  field: {{c.check_type}} = {{c.base_value}}
}

class A extends Base {
}

class B extends A {
  override field: {{c.check_type}} = {{c.override_value}}
}

function get_field(a: Base): {{c.check_type}} {
  return a.field
}

function set_field(a: Base, value: {{c.check_type}}) {
  a.field = value
}

function main(): void {
  let a: A = new B()
  arktest.assertTrue(a.field instanceof {{c.check_type}})
  arktest.assertEQ(a.field, {{c.override_value}})
  arktest.assertEQ(get_field(a), {{c.override_value}})
  set_field(a, {{c.new_value}})
  arktest.assertEQ(a.field, {{c.new_value}})
  arktest.assertEQ(get_field(a), {{c.new_value}})
}

{% endfor %}
