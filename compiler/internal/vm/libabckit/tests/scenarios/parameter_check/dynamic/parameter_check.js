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
import {
    Handler
} from './modules/base';

class StringHandler1 extends Handler {
    handle(arr, idx) {
        return arr[idx];
    }
}

class StringHandler2 extends Handler {
    handle(arr, idx) {
        return arr[idx];
    }
}

function foo(stringHandler, stringArray) {
    print(stringHandler.handle(stringArray, 0));
    print(stringHandler.handle(stringArray, 1));
    print(stringHandler.handle(stringArray, 2));
    print(stringHandler.handle(stringArray, 3));
}

foo(new StringHandler1(), ['str1', 'str2', 'str3']);
foo(new StringHandler2(), ['str3', 'str2', 'str4']);
