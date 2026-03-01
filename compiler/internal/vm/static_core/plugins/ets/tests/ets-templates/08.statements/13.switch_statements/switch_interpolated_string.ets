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
desc: Switch statement with interpolated string.
---*/

const foo: string = 'AbyrvalG'
let result = 'miss'

function sint() {
    switch(`abc${foo}cba`) {
        case 'abc': break
        case '123': break
        case `abc${foo}cba`: result = 'hit'
    }
}

function main() {
    result = 'miss'
    sint()
    arktest.assertEQ('hit', result)
}
