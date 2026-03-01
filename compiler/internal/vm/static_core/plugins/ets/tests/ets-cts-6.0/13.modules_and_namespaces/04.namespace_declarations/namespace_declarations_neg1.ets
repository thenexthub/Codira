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
desc: ES2PANDA_FAIL - TypeError Property 'nsConst' does not exist on type 'Space2'
tags: [compile-only, negative]
files:
- ./namespace_storage.ets
---*/

import {Space2 as mySpace} from "./namespace_storage"

function main() {
    if (mySpace.nsVar == mySpace.nsConst) {
        mySpace.foo()
    }
}
