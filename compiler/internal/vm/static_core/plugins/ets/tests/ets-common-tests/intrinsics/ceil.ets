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
    if (ceil(Double.POSITIVE_INFINITY) != Double.POSITIVE_INFINITY) {
        return 1;
    }
    if (ceil(Double.NEGATIVE_INFINITY) != Double.NEGATIVE_INFINITY) {
        return 2;
    }
    if (Double.toString(ceil(Double.NaN)) != Double.toString(Double.NaN)) {
        return 3;
    }
    if (ceil(0.0) != 0.0) {
        return 4;
    }
    if (ceil(1.0) != 1.0) {
        return 5;
    }
    if (ceil(-1.0) != -1.0) {
        return 6;
    }
    if (ceil(1.5) != 2.0) {
        return 7;
    }
    if (ceil(1.6) != 2.0) {
        return 8;
    }
    if (ceil(1.4) != 2.0) {
        return 9;
    }
    if (ceil(-1.5) != -1.0) {
        return 10;
    }
    if (ceil(-1.6) != -1.0) {
        return 11;
    }
    if (ceil(-1.4) != -1.0) {
        return 12;
    }
    return 0;
}
