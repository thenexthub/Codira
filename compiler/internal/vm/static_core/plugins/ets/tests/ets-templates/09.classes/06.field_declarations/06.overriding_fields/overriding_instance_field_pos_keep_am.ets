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
    An overriding member can keep or extend an access modifier
    of a member that is inherited or implemented.
    Otherwise, a compile-time error occurs.
---*/

class Base {
  protected field: {{c.check_type}} = {{c.base_value}}
  get_field() {
    return this.field
  }
}

class A extends Base {
  protected field: {{c.check_type}} = {{c.override_value}}
}

function get_field(a: Base): {{c.check_type}} {
  return a.get_field()
}

function main(): void {
  let a: A = new A()
  arktest.assertTrue(a.get_field() instanceof {{c.check_type}})
  arktest.assertEQ(a.get_field(), {{c.override_value}})
  arktest.assertEQ(get_field(a), {{c.override_value}})
}

{% endfor %}
