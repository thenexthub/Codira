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

{% for c in cases %}
/*---
desc: >-
    If thenStatement or elseStatement is any kind of a statement but not a block,
    then no block scope is created for such a statement.
---*/

function main(): void {
    if ({{c.cond}}) let x: number = 1;
    x = 2; // OK

    if ({{c.cond}}) {
        let x: number = 10; // OK, then-block scope
        let y: number = x;
    }
    else {
        let x: number = 20; // OK, no conflict, else-block scope
    }

    arktest.assertEQ(x, 2);
}
{% endfor %}
