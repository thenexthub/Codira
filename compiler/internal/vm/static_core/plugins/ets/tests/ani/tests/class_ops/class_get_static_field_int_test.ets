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
class TestInt {
    static int_value: int = 3;
    static string_value: String = "1024";
    static specia1: int = 0;
    static specia3: string = "";
    static specia4: null = null;
    static specia5: undefined  = undefined ;
    static min: int = -2147483648;
    static max: int = 2147483647;
}

class TestIntA extends TestInt {
    static int_value: int = 3;
}

final class TestIntFinal extends TestInt {
    static int_value: int = 3;
}

class Parent {
    static parentField: int = 1;
    static overridedField: int = 2;
}
class Child extends Parent {
    static overridedField: int = 4;
    static childField: int = 3;
}