/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

{% for c in cases %}
/*---
desc: Overload Resolution for Overload Declarations - method overload.
tags: []
---*/

final class A {
    max2i(a: int, b: int): int {
        return 100 + (a > b ? a : b);
    }

    max2d(a: double, b: double): double {
        return 200 + (a > b ? a : b);
    }

    maxN(...a: double[]): double {
        let max: double = a[0];
        for (let x: number of a) {
            if (x > max) {
                 max = x;
            }
        }
        return 300 + max;
    }

    overload maxFn { {{c.list}} };
}

function main() {
    let i: int = 1;
    let j: int = 2;
    let pi: number = 3.14;
    let a: A = new A();

    arktest.assertEQ({{c.result}}, a.{{c.call}});
}

{% endfor %}
