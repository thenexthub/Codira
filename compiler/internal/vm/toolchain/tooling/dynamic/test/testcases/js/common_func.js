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

function common_func() {
    var a = 'js_test';
    var b = false;
    var c = 10;
}

print("common_func test begin");
common_func();
print("common_func test end");

function for_loop() {
    for (var i = 0; i < 10; i++) {
        if (i == 3) break;
        print(i);
    }
    print("loop end");
}

function while_loop() {
    var i = 0;
    while (i < 10) {
        i++;
        if (i == 3) continue;
        print(i);
    }
    print("loop end");
}

function loop_switch() {
    for (var i = 0; i < 3; i++) {
        switch (i) {
            case 0: print(0);
                break;
            case 1: print(1);
                break;
            default:
                print("no this case");
        }
    }
}

function factorial(n) {
    if (n == 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

for_loop();
while_loop();
loop_switch();
factorial(4);
print("end");
