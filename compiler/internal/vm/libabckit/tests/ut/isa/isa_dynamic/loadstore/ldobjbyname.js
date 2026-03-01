/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

function foo() {
    let o = {
        field: 'field_data',
    };
    return 'just_data';
}

print(foo());



globalThis.then_cb = function(x) {
    print("then_cb");
    return x - 1;
}

function runNamedThen() {
    return Promise.resolve(41).then(
        //after aop, then_cb will replace this callback
        (x) => {
        print("123");
        return x + 2;
    }
);
}


runNamedThen()
    .then((v) => print("result:", v))
    .catch((e) => print("error:", e));