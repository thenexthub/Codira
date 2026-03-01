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

{% for c in cases %}
/*---
desc: |-
    A covariant type argument (or a wildcard with an upper bound) is signified by the following syntax, where T is a typeReference:
    out T
    It restricts available methods so it is only allowed to access methods that do not use T or do use ones with T in out-positions only.
    Consequently, a contravariant type argument (or a wildcard with a lower bound) is signified by the following syntax, where T is a typeReference:
    in T
    It restricts available methods so it is only allowed to access methods that do not use T or do use ones with T in in-positions only.
params: interface arguments, use-site variance
---*/

class X {}

class Y extends X {}

interface I<T> {
    methIn(t: T): void
    methOut(): T
    methInOut(t: T): T
}

class A<T> implements I<T> {
    private t: T

    public constructor(t: T) {
        this.t = t
    }

    public override methIn(t: T): void {
        this.t = t
    }

    public override methOut(): T {
        return this.t;
    }

    public override methInOut(t: T): T {
        this.t = t
        return this.t;
    }
}

function main() {

    {{c.use|indent}}

}
{% endfor %}
