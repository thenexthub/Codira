/*---
Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

{% for mod in modifiers %}

/*---
desc: Inheritance method of superclass.
assert: >-
    A class C inherits from its direct superclass all concrete methods m (both static and instance) of the superclass
    for which all of the following are true. m is a member of the direct superclass of C. m is public, protected,
    or declared with package access in the same package as C. No method declared in C has a signature that is a
    subsignature of the signature of m.
params: {{mod}}
---*/

class A {
  {{mod}} foo(): int {
    return 1;
  }
}

class B extends A {
  fee(): int {
    return this.foo();
  }
}

function main(): void {
  let instance: B = new B();

  arktest.assertEQ( instance.fee(), 1 );
}

{% endfor %}
