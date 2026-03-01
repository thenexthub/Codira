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
    Sample from spec, Namespaces with identical namespace names in a single compilation unit merge their exported
    declarations into a single namespace.
---*/

namespace A {
    let variable1 = 1;
    export namespace C {
        export let variable11 = 11;
    }
}

namespace A {
    let variable2 = 2;
}

namespace A.C {
    export let variable12 = 12;
}

function main() {
    arktest.assertEQ(A.C.variable11, 11);
    arktest.assertEQ(A.C.variable12, 12);
}
