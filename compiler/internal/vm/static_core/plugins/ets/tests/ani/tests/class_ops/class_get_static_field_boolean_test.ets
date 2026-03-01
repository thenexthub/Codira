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
class TestBoolean {
    static boolean_value: boolean = true;
    static string_value: String = "true";
    static special1: int = 0;
    static special3: String = "";
    static special4: null = null;
    static special5: undefined  = undefined ;
    static special6: boolean = !'s';
    static special7: boolean = !0;
    static special8: boolean = true;
    static special9: boolean = false;
}

class TestBooleanA extends TestBoolean {
    static boolean_value: boolean = false;
}

final class TestBooleanFinal extends TestBoolean {
    static boolean_value: boolean = false;
}