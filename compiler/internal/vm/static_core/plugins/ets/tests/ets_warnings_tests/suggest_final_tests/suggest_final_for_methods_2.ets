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

class I { // No final - inheritance
    aaa(): String { // No final - overrided
        return "a";
    };
}

final class A extends I { // No final - is already final
    something(): void {}; // // No final - methods of final class are already final
    override aaa(): String { // // No final - methods of final class are already final
        return "";
    }
}

final class K { // No final - is already final
    static foo(): int { // No final - static, methods of final class are already final
        return 10;
    };
}

interface H {
    fee(): void {}; // No final - abstract
}

function main(): int { // No final - ETSGLOBAL
    return 0;
}
