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
desc: A compile-time error occurs, if an overload alias is exported but an overloaded function is not.
tags: [compile-only, negative]
---*/

export function foo1(p: string): int {
    return 1;
}

export function foo2(p: number): int {
    return 2;
}

function foo3(p: boolean): int {
    return 3;
}

function foo4(p: number, q: boolean): int {
    return 4;
}

{{c.export}} overload foo { {{c.list}} }

{% endfor %}
