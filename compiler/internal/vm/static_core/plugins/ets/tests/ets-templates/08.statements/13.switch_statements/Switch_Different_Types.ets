/*---
Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
desc: Switch statement
---*/

type Switcher = String | Int | null


function foo(test: Switcher) {
    switch(test) {
        case null: { arktest.assertTrue(true); break }
        case 'Test': { arktest.assertTrue(true); break }
        case 5: { arktest.assertTrue(true); break }
        default: { arktest.assertTrue(false, 'Not expected to execute default branch') }
    }
}

function main() {
    foo('Test')
    foo(null)
    foo(5)
}
