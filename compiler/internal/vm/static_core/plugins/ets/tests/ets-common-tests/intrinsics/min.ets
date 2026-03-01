/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
    }
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

function main() : int {
    if (min(Int.MAX_VALUE as int, 0 as int) != 0) {
        return 1;
    }
    if (min(Int.MIN_VALUE as int, 0 as int) != Int.MIN_VALUE as int) {
        return 2;
    }
    if (min(0 as int, -2 as int) != -2 as int) {
        return 3;
    }
    if (min(0 as int, 3 as int) != 0 as int) {
        return 4;
    }
    if (min(-2 as int, -4 as int) != -4 as int) {
        return 5;
    }
    if (min(1 as int, 4 as int) != 1 as int) {
        return 6;
    }
    if (min(Long.MAX_VALUE, 0) != 0) {
        return 11;
    }
    if (min(Long.MIN_VALUE, 0) != Long.MIN_VALUE) {
        return 12;
    }
    if (min(0, -2) != -2) {
        return 13;
    }
    if (min(0, 3) != 0) {
        return 14;
    }
    if (min(-2, -4) != -4) {
        return 15;
    }
    if (min(1, 4) != 1) {
        return 16;
    }

    if (min(Float.POSITIVE_INFINITY, Float.NEGATIVE_INFINITY) != Float.NEGATIVE_INFINITY) {
        return 21;
    }
    if (min(Float.MAX_VALUE, Float.NEGATIVE_INFINITY) != Float.NEGATIVE_INFINITY) {
        return 22;
    }
    if (min(0.0f, Float.NEGATIVE_INFINITY) != Float.NEGATIVE_INFINITY) {
        return 23;
    }
    if (min(0.0f, Float.MIN_VALUE) != 0.0f) {
        return 24;
    }
    if (min(Float.MAX_VALUE, Float.MIN_VALUE) != Float.MIN_VALUE) {
        return 25;
    }
    if (min(0.0f, 2.0f) != 0.0f) {
        return 26;
    }
    if (min((0.0).toFloat(), (-4.0).toFloat()) != (-4.0).toFloat()) {
        return 27;
    }
    if (min((1.e-10).toFloat(), (2.e+12).toFloat()) != (1.e-10).toFloat()) {
        return 28;
    }

    if (min(Double.POSITIVE_INFINITY, Double.NEGATIVE_INFINITY) != Double.NEGATIVE_INFINITY) {
        return 31;
    }
    if (min(Double.MAX_VALUE, Double.NEGATIVE_INFINITY) != Double.NEGATIVE_INFINITY) {
        return 32;
    }
    if (min(0, Double.NEGATIVE_INFINITY) != Double.NEGATIVE_INFINITY) {
        return 33;
    }
    if (min(0, Double.MIN_VALUE) != 0) {
        return 34;
    }
    if (min(Double.MAX_VALUE, Double.MIN_VALUE) != Double.MIN_VALUE) {
        return 35;
    }
    if (min(0, 2) != 0) {
        return 36;
    }
    if (min(0, -4) != -4) {
        return 37;
    }
    if (min(1.e-100, 2.e+200) != 1.e-100) {
        return 38;
    }
    return 0;
}
