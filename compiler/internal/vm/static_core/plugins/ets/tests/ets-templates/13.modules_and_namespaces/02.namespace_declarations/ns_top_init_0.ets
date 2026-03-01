/*---
Copyright (c) 2026 Huawei Device Co., Ltd.
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
      Namespaces like modules can have top-level statements which act as namespace initializers, and are called only if at
      least one of the exported namespace members is used in the program

---*/
let accessCounter: int = 0
let init = false

namespace IAmUsed {
    export let someUsedVar: string
    accessCounter += 1
}

namespace NSUnused {
    export let someUnUsedVar: string
    accessCounter += 1
}

function main () {
    if(!init) {
        init = true;
        IAmUsed.someUsedVar = "Accessed 1"
        arktest.assertEQ(1, accessCounter)
        IAmUsed.someUsedVar = "Accessed 2"
        arktest.assertEQ(1, accessCounter)
    }
}
