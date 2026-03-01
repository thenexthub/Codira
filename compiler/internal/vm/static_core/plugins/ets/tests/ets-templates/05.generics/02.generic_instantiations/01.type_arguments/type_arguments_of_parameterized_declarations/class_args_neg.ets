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
desc: Type arguments
expected_out: {{c.stdout}}
params: class arguments, invariant
tags: [compile-only, negative]
---*/

interface I {}

class X implements I {}

class Y {}

enum Color { Red, Green, Blue }

enum Size { S, M, L, XL, XXL }

class A<T{{c.bound}}> {
    private t: T;

    public constructor(t: T) {
        this.t = t;
    }
}

{{c.decl}}

function main() {

    {{c.use|indent}}
    
}
{% endfor %}
