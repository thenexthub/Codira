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
desc: Throw statements cause an exception or Error to be thrown.
tags: []
---*/

class ThrowCatchThrow extends Error {}

function main() {
    try {
        try {
            throw new Error
        } catch(e) {
            throw new ThrowCatchThrow
        }
    } catch(e) {
        arktest.assertTrue( e instanceof ThrowCatchThrow )
        return;
    }
    arktest.assertFalse(true, "Not expected to be here");

}
