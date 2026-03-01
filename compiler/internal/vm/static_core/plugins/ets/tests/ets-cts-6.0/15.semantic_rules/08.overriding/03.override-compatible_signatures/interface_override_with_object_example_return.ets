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
    The example below illustrates override-compatibility with Object.
    If signatures are override-compatible, then overriding is used.
    Otherwise, a compile-time error occurs.
---*/

enum E1 {E1, E2}

interface Base {
    kinds_of_return_type<T extends Derived, U extends Base>(): Object
}

interface Derived extends Base {
    kinds_of_return_type<T extends Derived, U extends Base>(): {{c.type}}
}

class A implements Derived {
    res: int = 0

    kinds_of_return_type<T extends Derived, U extends Base>(): {{c.type}} {
        this.res = 2
        return {{c.expr}}
    }
}

function main() {
    let a = new A
    let v = a.kinds_of_return_type<Derived, Base>()
    arktest.assertEQ(a.res, 2)
}
{% endfor %}
