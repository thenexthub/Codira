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
    If a single accessor or both accessors are defined in an interface,
    then the accessors can be implemented as a field in a class
---*/

interface I {
  get prop(): {{c.check_type}}
  set prop(x: {{c.check_type}})
}

class A implements I {
  prop: {{c.check_type}} = {{c.implement_value}}
}

function get_prop(i: I): {{c.check_type}} {
  return i.prop
}

function set_prop(i: I, value: {{c.check_type}}) {
  i.prop = value
}

function main(): void {
  let a: A = new A()
  arktest.assertTrue(a instanceof A)
  arktest.assertTrue(a instanceof I)
  arktest.assertTrue(a.prop instanceof {{c.check_type}})
  arktest.assertEQ(a.prop, {{c.implement_value}})
  arktest.assertEQ(get_prop(a), {{c.implement_value}})
  set_prop(a, {{c.new_value}})
  arktest.assertEQ(a.prop, {{c.new_value}})
  arktest.assertEQ(get_prop(a), {{c.new_value}})
}

{% endfor %}
