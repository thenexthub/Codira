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

/*---
desc: >-
    A constructor body can have no more than one call to the current
    class or direct superclass constructor
tags: [compile-only, negative]
---*/

class A {
    public i: int

    public constructor(i: int) {
        this.i = i
    }
}

class B extends A {
    public s: string

    public constructor fromString(s: string) {
        this.s = s
    }

    public constructor() {
        this.fromString("Just a test")
        super(123)  // CTE: second call to superconstructor
    }
}
