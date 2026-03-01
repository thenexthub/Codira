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
desc: Conditional expression order
---*/

class Order {
    order = 0;
}

function op1(order: Order): void {
    order.order++
}

function op2(order: Order): void {
    order.order++
}

type LF = (flag: boolean, order: Order) => boolean


function main() {

    let o0 = new Order
    let o1 = new Order
    let o2 = new Order

    const op0: LF = (flag: boolean, order: Order) => { order.order++; return flag; }

    op0(false, o0) ? op1(o1) : op2(o2)

    arktest.assertEQ(1, o0.order)
    arktest.assertEQ(0, o1.order)
    arktest.assertEQ(1, o2.order)

    o0 = new Order
    o1 = new Order
    o2 = new Order

    op0(true, o0) ? op1(o1) : op2(o2)

    arktest.assertEQ(1, o0.order)
    arktest.assertEQ(1, o1.order)
    arktest.assertEQ(0, o2.order)

    
}
