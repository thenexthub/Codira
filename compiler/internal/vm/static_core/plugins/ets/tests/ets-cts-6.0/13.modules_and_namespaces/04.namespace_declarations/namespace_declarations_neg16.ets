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
    Transit namespace usage. TypeError Namespace 'Transit1' is already defined.
    TypeError Property 'foo' does not exist on type 'Transit1'
files:
- ./namespace_storage.ets
tags: [compile-only, negative]
---*/

import {Transit as Transit1} from "./namespace_storage"

export namespace Transit1.B {
    export let variable1 = 1;
}

function main() {
    arktest.assertEQ(Transit1.B.variable1, 1)
    arktest.assertEQ(Transit1.foo(), "A foo")
}
