/*
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
*/

/*---
desc: 15.5.1 Extended Conditional Expression
tags: []
---*/

type LambdaOneParam = (s: String) => boolean;

function getLambdaWithParam() : LambdaOneParam {
    let lambda: LambdaOneParam = (s: String) => {
        if (s) {
            return true;
        } else {
            return false;
        }
    }
    return lambda;
}


function main(): int {
    let l1 = getLambdaWithParam();
    let emptyString = "";
    if (l1(emptyString)) {
        return 1;
    } else {
        return 0
    }
}
