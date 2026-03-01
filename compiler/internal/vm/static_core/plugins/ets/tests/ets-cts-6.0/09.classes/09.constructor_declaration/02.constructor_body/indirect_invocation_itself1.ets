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

/*---
desc: Indirect invocation itself.
assert: >-
    It is a compile-time error for a constructor to directly or indirectly invoke itself through a series of one or
    more explicit constructor invocations involving this.
tags: [compile-only, negative]
---*/

class A {
    public x: int;

    public constructor oneArg(a: int) {
        this.twoArgs(5, 5)
    }

    public constructor twoArgs(a: int, b : int) {
        this.oneArg(b);
    }
}
