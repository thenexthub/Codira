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
desc: Issue 16657. Lambda constructor captures incorrect local register
tags: []
---*/

function isFinite(value: double) {
    return value >= Number.MIN_SAFE_INTEGER && value <= Number.MAX_SAFE_INTEGER
}

class TimeAnimationImpl<Value> {
    private compute: (time: long) => Value
    constructor(compute: (time: long) => Value) {
        this.compute = compute
    }
}

function smoothAnimation(period: int, from: float = 0.0f, to: float = 0.0f): TimeAnimationImpl<float> {
    if (!isFinite(period) || (period < 1)) throw new Error("illegal period: " + period)
    return new TimeAnimationImpl<float>((time:long): float => (1 - Math.cos(time / period * Math.PI).toFloat()) / 2 * (to - from) + from)
}


function main() {
    let tai = smoothAnimation(3)
}

