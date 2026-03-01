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

import {__$$ETS_ANNOTATION$$__Anno3, __$$ETS_ANNOTATION$$__Anno4} from "./annotations_imports"

@interface __$$ETS_ANNOTATION$$__Anno1 {
    a: number = 3
    b: number[] = [13, 9]
    d: boolean = false
}

@interface __$$ETS_ANNOTATION$$__Anno2 {
    a: number = 5
    b: number[] = [12, 4]
    d: boolean = true
}

@__$$ETS_ANNOTATION$$__Anno1({
    a: 20,
    b: [13, 10],
    d: true
})
class A {
    @__$$ETS_ANNOTATION$$__Anno1({
        a: 10,
        b: [1, 2, 3],
        d: true
    })
    foo() {}
    @__$$ETS_ANNOTATION$$__Anno1({
        a: 5,
        b: [1, 4]
    })
    bar() {}
}
