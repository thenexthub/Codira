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
    Checking static field declaration in derived class
    with same name and same type as in superclass.
params: hiding with default value
---*/

class Base {
  static field: {{c.check_type}} = {{c.base_value}}
}

class A extends Base {
  static field: {{c.check_type}}
}

function main(): void {
  arktest.assertTrue(A.field instanceof {{c.check_type}})
  arktest.assertEQ(A.field, {{c.derived_value}})
  arktest.assertTrue(Base.field instanceof {{c.check_type}})
  arktest.assertEQ(Base.field, {{c.base_value}})
}

{% endfor %}
