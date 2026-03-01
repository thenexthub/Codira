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
desc: Labeled statement
params: not related to loop
tags: [compile-only, negative]
---*/

function main(): void {
    const letters: ReadonlyArray<string> = ['a', 'b', 'c'];
    label: for(let letter of letters) {
        (() => { while(true) continue label })()  //  TypeError: Continue or break target can't be outside the function
    }
}
