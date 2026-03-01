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

function main() : int {
    if (max(Int.MAX_VALUE as int, 0 as int) != Int.MAX_VALUE as int) {
        return 1;
    }
    if (max(Int.MIN_VALUE as int, 0 as int) != 0 as int) {
        return 2;
    }
    if (max(0 as int, -2 as int) != 0 as int) {
        return 3;
    }
    if (max(0 as int, 3 as int) != 3 as int) {
        return 4;
    }
    if (max(-2 as int, -4 as int) != -2 as int) {
        return 5;
    }
    if (max(1 as int, 4 as int) != 4 as int) {
        return 6;
    }

    if (max(Long.MAX_VALUE, 0) != Long.MAX_VALUE) {
        return 11;
    }
    if (max(Long.MIN_VALUE, 0) != 0) {
        return 12;
    }
    if (max(0, -2) != 0) {
        return 13;
    }
    if (max(0, 3) != 3) {
        return 14;
    }
    if (max(-2, -4) != -2) {
        return 15;
    }
    if (max(1, 4) != 4) {
        return 16;
    }

    if (max(Float.POSITIVE_INFINITY, Float.NEGATIVE_INFINITY) != Float.POSITIVE_INFINITY) {
        return 21;
    }
    if (max(Float.MAX_VALUE, Float.NEGATIVE_INFINITY) != Float.MAX_VALUE) {
        return 22;
    }
    if (max(0.0f, Float.NEGATIVE_INFINITY) != 0) {
        return 23;
    }
    if (max(0.0f, Float.MIN_VALUE) != Float.MIN_VALUE) {
        return 24;
    }
    if (max(Float.MAX_VALUE, Float.MIN_VALUE) != Float.MAX_VALUE) {
        return 25;
    }
    if (max(0.0f, 2.0f) != 2.0f) {
        return 26;
    }
    if (max((0.0).toFloat(), (-4.0).toFloat()) != (0.0).toFloat()) {
        return 27;
    }
    if (max((1.e-10).toFloat(), (2.e+12).toFloat()) != (2.e+12).toFloat()) {
        return 28;
    }

    if (max(Double.POSITIVE_INFINITY, Double.NEGATIVE_INFINITY) != Double.POSITIVE_INFINITY) {
        return 31;
    }
    if (max(Double.MAX_VALUE, Double.NEGATIVE_INFINITY) != Double.MAX_VALUE) {
        return 32;
    }
    if (max(0, Double.NEGATIVE_INFINITY) != 0) {
        return 33;
    }
    if (max(0, Double.MIN_VALUE) != Double.MIN_VALUE) {
        return 34;
    }
    if (max(Double.MAX_VALUE, Double.MIN_VALUE) != Double.MAX_VALUE) {
        return 35;
    }
    if (max(0, 2) != 2) {
        return 36;
    }
    if (max(0, -4) != 0) {
        return 37;
    }
    if (max(1.e-100, 2.e+200) != 2.e+200) {
        return 38;
    }
    return 0;
}
