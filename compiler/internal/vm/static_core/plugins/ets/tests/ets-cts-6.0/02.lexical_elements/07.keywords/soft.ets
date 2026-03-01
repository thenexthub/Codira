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

{% for kwd in keywords %}
/*---
desc: The following soft keywords have special meaning in certain contexts but are valid identifiers elsewhere
params: '{{kwd}}'
---*/

let {{kwd}} = 0;
{{kwd}} = 1

class A {
    {{kwd}} = 1
}

class B {
    readonly {{kwd}} = 1
}

interface I{
    test({{kwd}}: String): String; 
}

class C implements I{
    test({{kwd}}: String): String{
        return {{kwd}};
    }
}

abstract class AD {
    readonly {{kwd}} = 1
    abstract test({{kwd}}: String): String;
}

class D extends AD{
    test({{kwd}}: String): String{
        return {{kwd}};
    }
}

function main(): void{
    arktest.assertEQ({{kwd}}, 1)
    let ca = new A()
    ca.{{kwd}} = 2
    arktest.assertEQ(ca.{{kwd}}, 2)
    let cb = new B() 
    arktest.assertEQ(cb.{{kwd}}, 1)
    let cc = new C()
    arktest.assertEQ(cc.test("test"), "test")
    let cd = new D()
    arktest.assertEQ(cd.test("test"), "test")
    arktest.assertEQ(cd.{{kwd}}, 1)
}

{% endfor %}
