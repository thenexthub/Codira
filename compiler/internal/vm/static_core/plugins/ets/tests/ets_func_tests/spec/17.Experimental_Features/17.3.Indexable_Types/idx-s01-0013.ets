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
 tags: [compile-only,negative]
---*/


class Bar {
    private data : int;
    private data2: int;
    private data3: int;

    $_get(index: int) : int {
          return this.data;
    }

    async $_set(index: int, value: int) {
         this.data = value;
    }

    constructor(data : int, data2: int, data3: int) {
        this.data = data;
        this.data2 = data2;
        this.data3 = data3;
    }
}

function main(): int {
    let x : Bar = new Bar(1, 2, 3);

}
