/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

/*---
desc: Multiple inheritance conflict resolving.
---*/

let a: number = 10;
let res: string = ""

interface A {
    draw(): void;
}

interface B {
    draw(): string;
}

interface C {
    draw(x: number): void;
}

interface D {
    draw(x: string): void;
}

class E implements A, B, C, D {
    draw(...p: Any[]): Any {
        if (p.length == 0) {
            if (a == 10)
                return "B"
            res = "B"
        else if (p[0] instanceof number)
            res = "C"
        else if (p[0] instanceof number)
            res = "D"
        }
        return
    }
}

function main() {
    res = ""
    a = 10
    let c: E = new E()
    arktest.assertEQ(c.draw(), "B")
    a++
    c.draw()
    arktest.assertEQ(res, "A")
    c.draw(1)
    arktest.assertEQ(res, "C")
    c.draw("str")
    arktest.assertEQ(res, "D")
}