/*---
Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

{% for case in cases %}

/*---
desc: >-
    The result of the field access expression is computed at runtime.
    For a non-static field: The object reference expression is evaluated.
    Only the object reference type (not the class type of an actual object referred at runtime)
    is used to determine the field to be accessed
---*/

interface IName {
    name: String;
}

class A {
  private _a: {{case.type}} = {{case.value}};

  get a(): {{case.type}}{
    return this._a;
  }
}

class B extends A {
}

final class C extends B {
  b: {{case.type}} = super.a;
}

function main(): void {
  let instance: C = new C;
  arktest.assertTrue( {{case.assert}} )
}

{% endfor %}
