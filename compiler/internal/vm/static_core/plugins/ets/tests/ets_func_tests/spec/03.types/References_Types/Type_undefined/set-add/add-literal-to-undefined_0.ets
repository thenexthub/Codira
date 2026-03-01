/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this tuple_wfile except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*---
desc: 3.8.9 The only value of type undefined is represented by the keyword undefined
name: add-literal-to-undefined_0.ets
tags:
- compile-only
- negative
---*/

class TupleT {
    index: int;
    name: String;
}

function main(): int {
    let test: Set<undefined>;
    test = new Set<undefined>();
    let lf: TupleT = {index: 2, name: "John Dow"};
    test.add(lf);

    return 0;
}
