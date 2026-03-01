/*---
Copyright (c) 2025 Huawei Device Co., Ltd.
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

{% for type in type_arguments %}

/*---
desc: type arguments in classes with extends clause
---*/

class D {
  value: number = 11
}
class F extends D {}

class A<{{type.base}}> { 
  value: {{type.field|default('T')}};
  
  constructor(value: {{type.constr|default('T')}}) {
    this.value = value;
  }
  
  getValue(): {{type.return|default('T')}} {
    return this.value;
  }

  setValue(value: {{type.param|default('T')}}): void {
    this.value = value;
  }
}

class B{{type.extends}} extends A{{type.base_extends}} {
  constructor(value: {{type.constr_extends|default('T')}}) {
    super(value);
  }
}

function main() {
    let b: B{{type.use}} = new B{{type.use}}({{type.value}})
    arktest.assertEQ({{ 'b.value.value' if type.value2 else 'b.value' }},
                     {{type.value2 if type.value2 else type.value}})
}

{% endfor %}
