/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
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
 desc: 17.3 Indexable Types
 tags:
 ---*/

class Bar<T, I> {
    private data : T
    $_get (index: I) : T {return this.data}
    $_set (index: I, value: T)  {this.data = value}

    constructor(data : T) {
        this.data = data;
    }
}

function main() {
    let x : Bar<Int, string> = new Bar<Int, string>(1);
    console.log("1");
    arktest.assertEQ(x["1"], 1)
    x["1"] = 2;
    arktest.assertEQ(x["1"], 2)
    return 0;
}
