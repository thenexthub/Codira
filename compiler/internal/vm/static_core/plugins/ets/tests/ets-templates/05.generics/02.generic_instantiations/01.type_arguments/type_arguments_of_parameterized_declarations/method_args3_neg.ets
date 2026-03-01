/*---
Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
desc: It is a compile-time error if a wildcard is used in a parameterization of a function, method, or constructor.
params: method, use-site variance
tags: [compile-only, negative]
expected_out: {{c.stdout}}
---*/

class X {}

class A {
    private v: X

    public constructor(v: X) {
        this.v = v
    }

    public meth1 <V extends X> (): V {
        return this.v
    }

    public meth2 <V extends X> (v: V): void {
        this.v = v
    }
}

function main() {

    let a: A = new A(new X())
    {{c.use|indent}}
    
}
{% endfor %}
