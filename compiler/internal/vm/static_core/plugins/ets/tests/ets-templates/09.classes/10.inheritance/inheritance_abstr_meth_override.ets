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
desc: Inheritance method multiple level with overriding
---*/

class E {
    e(): string { return 'e' }
}
class F extends E {
    e(): string { return 'f' }
}
class K extends F {
    e(): string { return 'k' }
}

{% if c.abstract %}
abstract class A {
    abstract a(f: {{c.param_a}}): {{c.ret_a}};
}
{% else %}
class A {
    a(f: {{c.param_a}}): {{c.ret_a}} { 
        return new {{c.ret_a}}() 
    }
}
{% endif %}

abstract class B extends A {
    abstract a(f: {{c.param_b}}): {{c.ret_b}};
}

class C extends B {
    a(f: {{c.param_c}}): {{c.ret_c}} {
        return {{c.impl}}
    }
}

function main() {
    const c: C = new C()
    const e: E = c.a({{c.call}})
    arktest.assertEQ(e.e(), {{c.expected}});
    arktest.assertTrue(e instanceof {{c.ret_c}})
}
{% endfor %}