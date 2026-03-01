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
desc: ES2PANDA_FAIL - TypeError Function boo is already declared. !!!Pay attention this works in TS!!!
tags: [compile-only, negative]
---*/

namespace B {
    function boo() {
        return "Another BOO"
    }
}

namespace B {
    function boo() {
        return "Boo"
    }
}
