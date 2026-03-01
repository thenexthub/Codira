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

class Bar {
    private data : string []
    $_get (index: int) : string {return this.data[index]}
    $_set (index: int, value: string)  {this.data[index] = value}

    constructor(data : string[]) {
        this.data = data;
    }
}

function main() {
    let x : Bar = new Bar(["1", "2", "3"]);
    arktest.assertEQ(x[0], "1")
    arktest.assertEQ(x[1], "2")
    arktest.assertEQ(x[2], "3")

    x[0] = "4";
    x[1] = "5";
    x[2] = "6";

    arktest.assertEQ(x[0], "4")
    arktest.assertEQ(x[1], "5")
    arktest.assertEQ(x[2], "6")
    return 0;
}
