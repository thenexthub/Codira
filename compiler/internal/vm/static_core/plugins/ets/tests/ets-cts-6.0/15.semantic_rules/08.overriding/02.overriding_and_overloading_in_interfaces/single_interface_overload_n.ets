/*---
Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
desc: Interface method overload signature cannot have a body.
tags: [negative, compile-only]
---*/

type TPL = [boolean, string];
type BoolFun = () => boolean;

interface I {
    foo(i: {{case}})
    foo(j: {{case}}) {} // CTE: Interface body is not allowed
}

class A implements I {
    foo(...p: Any[]): Any {
        return
    }
}

function main(): void {
  let a: A = new A();
}

{% endfor %}
