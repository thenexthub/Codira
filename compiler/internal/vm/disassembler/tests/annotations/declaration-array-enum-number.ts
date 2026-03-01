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

const enum E {
    A = 42,
    B = -314
} 

@interface __$$ETS_ANNOTATION$$__Anno1 {
    // without initializer in .ets source code with underlying number type
    a: E[] = new Array<E>(0);
}

@interface __$$ETS_ANNOTATION$$__Anno2 {
    // with empty array initializer in .ets source code with underlying number type
    a: E[] = new Array<E>(1);
}

@interface __$$ETS_ANNOTATION$$__Anno3 {
    a: E[] = [42, -314, 42];
}

