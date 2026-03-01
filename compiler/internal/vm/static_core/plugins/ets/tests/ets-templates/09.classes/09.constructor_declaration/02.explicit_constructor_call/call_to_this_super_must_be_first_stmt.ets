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
desc: >-
    A constructor body can have no more than one call to the current
    class or direct superclass constructor. Otherwise, a compile-time
    error occurs.
tags: [compile-only, negative]
---*/

class A {
    protected parentField: int

    protected parentMethod(): int {
        return 1
    }

    public constructor() {}

    public constructor fromString(value: string) {}
}

class B extends A {
    childField1: int
    childField2: string

    childMethod1(): int {
        return this.childField1
    }

    childMethod2(): string {
        return this.childField2
    }

    public constructor fromInt(value: int) {
        this.childField1 = value
    }

    public constructor fromString(value: String) {
        this.childField2 = value
    }

    public constructor() {
        {{c.stmt|indent|indent}}
    }
}
{% endfor %}
