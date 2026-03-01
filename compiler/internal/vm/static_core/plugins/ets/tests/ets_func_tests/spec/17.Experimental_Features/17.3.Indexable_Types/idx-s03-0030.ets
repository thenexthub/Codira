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
    private data1 : int;

    $_get(index: int) : int {
        switch (index) {
          case 0:
            return this.data1;
          default:
            throw new Error("Hmm ...");
        }
    }

    $_set(index: int, value: int) {
        switch (index) {
          case 0:
            this.data1 = value;
            break;
          default:
            throw new Error("Hmm ...");
        }
    }

    $_get(index: string) : int {
        switch (index) {
          case "hello":
            return this.data1;
          default:
            throw new Error("Hmm ...");
        }
    }

    $_set(index: string, value: int) {
        switch (index) {
          case "hello":
            this.data1 = value;
            break;
          default:
            throw new Error("Hmm ...");
        }
    }

    constructor(data : int) {
        this.data1 = data;
    }
}



function test1() : int {
    let result = 0;
    let x = new Bar(10);

    try {
        arktest.assertEQ(x[0], 10)
    } catch (e) {
        result++
        console.log("Unexpexted error 1");
    }

    try {
        arktest.assertEQ(x["hello"], 10)
    } catch (e) {
        result++
        console.log("Unexpexted error 1");
    }

    try {
        x[0] = 100
    } catch (e) {
        result++
        console.log("Unexpexted error 3");
    }

    try {
        arktest.assertEQ(x[0], 100)
    } catch (e) {
        result++
        console.log("Unexpexted error 4");
    }

    try {
        x["hello"] = 100
    } catch (e) {
        result++
        console.log("Unexpexted error 5");
    }

    try {
        arktest.assertEQ(x["hello"], 100)
    } catch (e) {
        result++
        console.log("Unexpexted error 6");
    }

    return result;
}

function test2() : int {
    let result = 0;
    let x = new Bar(10);
    try {
        arktest.assertEQ(x[10], 1)
    } catch (e) {
        result++
    }

    try {
        arktest.assertEQ(x["bye"], 1)
    } catch (e) {
        result++
    }


    return result;
}

function main() {
    arktest.assertEQ(test1(), 0)
    arktest.assertEQ(test2(), 2)
}

