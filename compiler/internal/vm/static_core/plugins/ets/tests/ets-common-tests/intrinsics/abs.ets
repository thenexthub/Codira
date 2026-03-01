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

function main(): int {
    if (abs(Double.POSITIVE_INFINITY) != Double.POSITIVE_INFINITY) {
        return 3;
    }
    if (abs(Double.NEGATIVE_INFINITY) != Double.POSITIVE_INFINITY) {
        return 4;
    }
    if (abs(Double.MAX_VALUE) != Double.MAX_VALUE) {
        return 5;
    }
    if (abs(-Double.MAX_VALUE) != Double.MAX_VALUE) {
        return 6;
    }
    if (abs(Double.MIN_VALUE) != Double.MIN_VALUE) {
        return 7;
    }
    if (abs(-Double.MIN_VALUE) != Double.MIN_VALUE) {
        return 8;
    }
    if (abs(1.0) != 1.0) {
        return 9;
    }
    if (abs(-1.0) != 1.0) {
        return 10;
    }
    if (abs(0.0) != 0.0) {
        return 11;
    }
    return 0;
}
