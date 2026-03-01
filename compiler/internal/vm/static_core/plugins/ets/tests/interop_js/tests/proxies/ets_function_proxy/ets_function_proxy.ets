/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

package function_proxy.test;


class Point {
    constructor(x: double, y: double) {
        this.x = x;
        this.y = y;
    }
    public x: double;
    public y: double;
}

function point_create(x: double, y: double): Point {
    return new Point(x, y)
}

function point_get_x(p: Point): double {
    return p.x;
}

function point_get_y(p: Point): double {
    return p.y;
}


function sum_double(v0: double, v1: double): double {
    return v0 + v1;
}
